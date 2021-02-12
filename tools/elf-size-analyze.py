#!/usr/bin/env python3
#
# Copyright (c) 2016, Intel Corporation
#
# SPDX-License-Identifier: Apache-2.0

# Based on a script by:
#       Chereau, Fabien <fabien.chereau@intel.com>

# ^originial comments before my modifications
# This script is based on 'size_report' from Zephyr Project scripts:
#   https://github.com/zephyrproject-rtos/zephyr/blob/master/scripts/footprint/size_report
#
# It has been modified to be more flexible for different (also not-bare-metal) ELF files,
# and adds some more data visualization options. Parsing has been updated to use
# regular expressions as it is much more robust solution.

import os
import re
import sys
import math
import shutil
import logging
import pathlib
import argparse
import itertools
import subprocess
import platform


# default logging configuration
log = logging.getLogger('elf-size-analyze')
console = logging.StreamHandler()
formatter = logging.Formatter('[%(levelname)s] %(message)s')
console.setFormatter(formatter)
log.setLevel(logging.ERROR)
log.addHandler(console)


def parse_args():
    parser = argparse.ArgumentParser(description="""
Prints report of memory usage of the given executable.
Shows how different source files contribute to the total size.
Uses inforamtion contained in ELF executable and binutils programs.
For best results the program should be compiled with maximum debugging information
(e.g. GCC flag: `-g`, or more: `-ggdb3`).
    """, epilog="""
This script is based on 'size_report' script from Zephyr Project:
https://github.com/zephyrproject-rtos/zephyr (scripts/footprint/size_report).
    """)

    parser.add_argument('elf', metavar='ELF_FILE',
                        help='path to the examined ELF file')

    memory_group = parser.add_argument_group(
        'Memory type', """
Specifies memory types for which statistics should be printed.
Choosing at least one of these options is required.
RAM/ROM options may be oversimplifed for some targets, under the hood they just filter the symbols
by sections in the following manner:
sections must have ALLOC flag and: for RAM - have WRITE flag, for ROM - not have NOBITS type.
        """)
    memory_group.add_argument('-R', '--ram', action='store_true',
                              help='print RAM statistics')
    memory_group.add_argument('-F', '--rom', action='store_true',
                              help='print ROM statistics ("Flash")')
    memory_group.add_argument('-P', '--print-sections', action='store_true',
                              help='print section headers that can be used for filtering symbols with -S option'
                              + ' (output is almost identical to `readelf -WS ELF_FILE`)')
    memory_group.add_argument('-S', '--use-sections', nargs='+', metavar='NUMBER',
                              help='manually select sections from which symbols will be used (by number)')

    basic_group = parser.add_argument_group(
        'Basic arguments')
    basic_group.add_argument('-t', '--toolchain-triplet', '--toolchain-path',
                             default='', metavar='PATH',
                             help='toolchain triplet/path to prepend to binutils program names,'
                             + ' this is important for examining cross-compiled ELF files,'
                             + ' e.g `arm-none-eabi-` or `/my/path/arm-none-eabi-` or `/my/path/`')
    basic_group.add_argument('-v', '--verbose', action='count',
                             help='increase verbosity, can be specified up to 3 times'
                             + ' (versobity levels: ERROR -> WARNING -> INFO -> DEBUG)')

    printing_group = parser.add_argument_group(
        'Printing options', 'Options for changing the output formatting.')
    printing_group.add_argument('-w', '--max-width', default=80, type=int,
                                help='set maximum output width, 0 for unlimited width (default 80)')
    printing_group.add_argument('-m', '--min-size', default=0, type=int,
                                help='do not print symbols with size below this value')
    printing_group.add_argument('-f', '--fish-paths', action='store_true',
                                help='when merging paths, use fish-like method to shrink them')
    printing_group.add_argument('-s', '--sort-by-name', action='store_true',
                                help='sort symbols by name instead of sorting by size')
    printing_group.add_argument('-H', '--human-readable', action='store_true',
                                help='print sizes in human readable format')
    printing_group.add_argument('-o', '--files-only', action='store_true',
                                help='print only files (to be used with cumulative size enabled)')
    printing_group.add_argument('-a', '--alternating-colors', action='store_true',
                                help='use alternating colors when printing symbols')

    printing_group.add_argument('--no-demangle', action='store_true',
                                help='disable demangling of C++ symbol names')
    printing_group.add_argument('--no-merge-paths', action='store_true',
                                help='disable merging paths in the table')
    printing_group.add_argument('--no-color', action='store_true',
                                help='disable colored output')
    printing_group.add_argument('--no-cumulative-size', action='store_true',
                                help='disable printing of cumulative sizes for paths')
    printing_group.add_argument('--no-totals', action='store_true',
                                help='disable printing the total symbols size')

    args = parser.parse_args()

    return args


################################################################################

class TreeNode:
    """
    Simple implementation of a tree with dynamic number of nodes.
    Provides a depth-first iterator. Someone could actually call this
    class TreeNode, as every object represents a single node.
    """

    def __init__(self, parent=None):
        self.parent = parent
        self.children = []

    def add(self, children):
        if not isinstance(children, (list, tuple)):
            children = (children, )
        for child in children:
            self.children.append(child)
            child.parent = self

    def pre_order(self):
        """Iterator that yields tuples (node, depth). Depth-first, pre-order traversal."""
        return self.PreOrderIterator(self)

    def post_order(self):
        """Iterator that yields tuples (node, depth). Depth-first, post-order traversal."""
        return self.PostOrderIterator(self)

    def __iter__(self):
        for child in self.children:
            yield child

    class TreeIterator:
        def __init__(self, root, depth=0):
            self.root = root
            self.depth = depth

        def __iter__(self):
            raise NotImplementedError('Should yield pairs (node, depth)')

    # depth-first tree iterators
    class PreOrderIterator(TreeIterator):
        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)

        def __iter__(self):
            yield self.root, self.depth
            children_iters = map(lambda child:
                                 self.__class__(child, self.depth + 1), self.root)
            for node in itertools.chain(*children_iters):
                yield node

    class PostOrderIterator(TreeIterator):
        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)

        def __iter__(self):
            children_iters = map(lambda child:
                                 self.__class__(child, self.depth + 1), self.root)
            for node in itertools.chain(*children_iters):
                yield node
            yield self.root, self.depth


# only for testing the implementation
def test__TreeNode():
    class NameTree(TreeNode):
        def __init__(self, name, *args, **kwargs):
            self.name = name
            super().__init__(*args, **kwargs)

        def __repr__(self):
            return 'Node(%s)' % self.name

    def create_tree():
        root = NameTree('root')
        root.add([NameTree('n1'), NameTree('n2'), NameTree('n3')])
        root.children[0].add([NameTree('n1n1'), NameTree('n1n2'), NameTree('n1n3')])
        root.children[1].add([NameTree('n2n1'), NameTree('n2n2')])
        root.children[2].add([NameTree('n3n1')])
        root.children[2].children[0].add([NameTree('n3n1n1')])
        return root

    root = create_tree()
    print('\nIterate over a node (root node):')
    for node in root:
        print('    |%s' % node)

    methods = [TreeNode.pre_order, TreeNode.post_order]
    for method in methods:
        print('\nIterate over tree (%s):' % method.__name__)
        for node, depth in method(root):
            print('    |%s%-30s  parent=%s' % ('    ' * depth, node, node.parent))

    sys.exit(0)


#  test__TreeNode()


################################################################################

class Color:
    """
    Class for easy color codes manipulations.
    """

    _base_string = '\033[%sm'
    _colors = {
        'BLACK':   0,
        'RED':     1,
        'GREEN':   2,
        'YELLOW':  3,
        'BLUE':    4,
        'MAGENTA': 5,
        'CYAN':    6,
        'GRAY':    7,
    }

    def __init__(self, color_codes=[]):
        try:
            self.color_codes = set(color_codes)
        except TypeError:
            self.color_codes = set([color_codes])

    def __add__(self, other):
        if isinstance(other, Color):
            return Color(self.color_codes.union(other.color_codes))
        elif isinstance(other, str):
            return str(self) + other
        return NotImplemented

    def __radd__(self, other):
        if isinstance(other, str):
            return other + str(self)
        return NotImplemented

    def __str__(self):
        return self._base_string % ';'.join(str(c) for c in self.color_codes)

    def __repr__(self):
        return 'Color(%s)' % self.color_codes


# should probably be done in a metaclass or something
for name, value in Color._colors.items():
    # regular color
    setattr(Color, name, Color(value + 30))
    # lighter version
    setattr(Color, 'L_%s' % name, Color(value + 90))
    # background
    setattr(Color, 'BG_%s' % name, Color(value + 40))
    # lighter background
    setattr(Color, 'BG_L_%s' % name, Color(value + 100))

setattr(Color, 'RESET', Color(0))
setattr(Color, 'BOLD', Color(1))
setattr(Color, 'DIM', Color(2))
setattr(Color, 'UNDERLINE', Color(4))
setattr(Color, 'BLINK', Color(5))
setattr(Color, 'REVERSE', Color(7))  # swaps background and forground
setattr(Color, 'HIDDEN', Color(8))


def test__colors():
    for attr in dir(Color):
        if attr.isupper() and not attr.startswith('_'):
            print(getattr(Color, attr) + 'attribute %s' % attr + Color.RESET)
    sys.exit(0)


#  test__colors()


################################################################################

# construct python regex named group
def g(name, regex):
    return r'(?P<{}>{})'.format(name, regex)


# print human readable size
# https://stackoverflow.com/questions/1094841/reusable-library-to-get-human-readable-version-of-file-size
def sizeof_fmt(num, suffix='B'):
    for unit in ['', 'Ki', 'Mi', 'Gi', 'Ti', 'Pi', 'Ei', 'Zi']:
        if abs(num) < 1024.0:
            suffix_str = unit + suffix
            return "%3.1f %-3s" % (num, suffix_str)
        num /= 1024.0
    unit = 'Yi'
    suffix_str = unit + suffix
    return "%3.1f %-3s" % (num, suffix_str)


################################################################################

class Symbol:
    """
    Represents a linker symbol in an ELF file. Attributes are as in the output
    of readelf command. Additionally, has optional file path and line number.
    """

    def __init__(self, num, name, value, size, type, bind, visibility, section,
                 file=None, line=None):
        self.num = num
        self.name = name
        self.value = value
        self.size = size
        self.type = type
        self.bind = bind
        self.visibility = visibility
        self.section = section
        self.file = file
        self.line = line

    def __repr__(self):
        return 'Symbol(%s)' % (self.name, )

    # Regex for parsing readelf output lines
    # Readelf output should look like the following:
    #   Symbol table '.symtab' contains 623 entries:
    #      Num:    Value  Size Type    Bind   Vis      Ndx Name
    #        0: 00000000     0 NOTYPE  LOCAL  DEFAULT  UND
    #   ...
    #      565: 08002bf9     2 FUNC    WEAK   DEFAULT    2 TIM2_IRQHandler
    #      566: 200002a8    88 OBJECT  GLOBAL DEFAULT    8 hspi1
    pattern_fields = [
        r'\s*',
        g('num', r'\d+'), r':',
        r'\s+',
        g('value', r'[0-9a-fA-F]+'),
        r'\s+',
        g('size', r'[0-9]+'),
        r'\s+',
        g('type', r'\S+'),
        r'\s+',
        g('bind', r'\S+'),
        r'\s+',
        g('visibility', r'\S+'),
        r'\s+',
        g('section', r'\S+'),
        r'\s+',
        g('name', r'.*'),
    ]
    pattern = r'^{}$'.format(r''.join(pattern_fields))
    pattern = re.compile(pattern)

    @classmethod
    def from_readelf_line(cls, line,
                          ignored_types=['NOTYPE', 'SECTION', 'FILE'],
                          ignore_zero_size=True):
        """
        Create a Symbol from a line of `readelf -Ws` output.
        """
        m = cls.pattern.match(line)
        if not m:
            log.debug('no match: ' + line.strip())
            return None

        # convert non-string values
        m = m.groupdict()
        m['num'] = int(m['num'])
        m['value'] = int(m['value'], 16)
        m['size'] = int(m['size'])  # suprisingly in decimal
        try:  # for numeric sections
            m['section'] = int(m['section'])
        except ValueError:
            pass

        # ignore if needed
        if not m['name'].strip() \
                or m['type'].lower() in map(str.lower, ignored_types) \
                or (ignore_zero_size and m['size'] == 0):
            log.debug('ignoring: ' + line.strip())
            return None

        # create the Symbol
        s = Symbol(**m)

        return s

    @classmethod
    def extract_elf_symbols_info(cls, elf_file, readelf_exe='readelf'):
        """
        Uses binutils 'readelf' to find info about all symbols from an ELF file.
        """
        flags = ['--wide', '--syms']
        readelf_proc = subprocess.Popen([readelf_exe, *flags, elf_file],
                                        stdout=subprocess.PIPE, universal_newlines=True)

        # parse lines
        log.info('Using readelf symbols regex: %s' % cls.pattern.pattern)
        symbols = [Symbol.from_readelf_line(l) for l in readelf_proc.stdout]
        n_ignored = len(list(filter(lambda x: x is None, symbols)))
        symbols = list(filter(None, symbols))

        if readelf_proc.wait(3) != 0:
            raise subprocess.CalledProcessError(readelf_proc.returncode,
                                                readelf_proc.args)

        log.info('ignored %d/%d symbols' % (n_ignored, len(symbols) + n_ignored))

        return symbols


def extract_elf_symbols_fileinfo(elf_file, nm_exe='nm'):
    """
    Uses binutils 'nm' to find files and lines where symbols from an ELF
    executable were defined.
    """
    # Regex for parsing nm output lines
    # We use Posix mode, so lines should be in form:
    #   NAME TYPE VALUE SIZE[\tFILE[:LINE]]
    # e.g.
    #   MemManage_Handler T 08004130 00000002	/some/path/file.c:80
    #   memset T 08000bf0 00000010
    pattern_fields = [
        g('name', r'\S+'),
        r'\s+',
        g('type', r'\S+'),
        r'\s+',
        g('value', r'[0-9a-fA-F]+'),
        r'\s+',
        g('size', r'[0-9a-fA-F]+'),
        g('fileinfo', r'.*'),
    ]
    pattern = r'^{}$'.format(r''.join(pattern_fields))
    pattern = re.compile(pattern)
    log.info('Using nm symbols regex: %s' % pattern.pattern)

    # use posix format
    flags = ['--portability', '--line-numbers']
    nm_proc = subprocess.Popen([nm_exe, *flags, elf_file],
                               stdout=subprocess.PIPE, universal_newlines=True)

    # process nm output
    fileinfo_dict = {}
    for line in nm_proc.stdout:
        m = pattern.match(line)
        if not m:
            continue

        # parse the file info
        file, line = None, None
        fileinfo = m.group('fileinfo').strip()
        if len(fileinfo) > 0:
            # check for line number
            line_i = fileinfo.rfind(':')
            if line_i >= 0:
                file = fileinfo[:line_i]
                line = int(fileinfo[line_i + 1])
            else:
                file = fileinfo
            # try to make the path more readable
            file = os.path.normpath(file)

        fileinfo_dict[m.group('name')] = file, line

    if nm_proc.wait(3) != 0:
        raise subprocess.CalledProcessError(nm_proc.returncode,
                                            nm_proc.args)

    return fileinfo_dict


def add_fileinfo_to_symbols(fileinfo_dict, symbols_list):
    # use dictionary for faster access (probably)
    symbols_dict = {s.name: s for s in symbols_list}
    for symbol_name, (file, line) in fileinfo_dict.items():
        if file is None and line is None:
            continue
        if symbol_name in symbols_dict:
            symbol = symbols_dict[symbol_name]
            symbol.file = file
            symbol.line = line
        else:
            log.warning('nm found fileinfo for symbol "%s", which has not been found by readelf'
                            % symbol_name)


def demangle_symbol_names(symbols, cppfilt_exe='c++filt'):
    """
    Use c++filt to demangle symbol names in-place.
    """
    flags = []
    cppfilt_proc = subprocess.Popen(
        [cppfilt_exe, *flags], stdin=subprocess.PIPE, stdout=subprocess.PIPE, universal_newlines=True)

    for symbol in symbols:
        # write the line and flush it
        # not super-efficient but writing all at once for large list of symbols
        # can block the program (probably due to buffering)
        cppfilt_proc.stdin.write((symbol.name + '   \n'))
        cppfilt_proc.stdin.flush()
        new_name = cppfilt_proc.stdout.readline().strip()
        symbol.name = new_name
    cppfilt_proc.stdin.close()

    if cppfilt_proc.wait(3) != 0:
        raise subprocess.CalledProcessError(cppfilt_proc.returncode,
                                            cppfilt_proc.args)


# Some nice info about sections in ELF files:
# http://www.sco.com/developers/gabi/2003-12-17/ch4.sheader.html#sh_flags
class Section:
    """Represents an ELF file section as read by `readelf -WS`."""

    # Regex for parsing readelf sections information
    # Example output:
    #   Section Headers:
    #     [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
    #     [ 0]                   NULL            00000000 000000 000000 00      0   0  0
    #     [ 1] .isr_vector       PROGBITS        08000000 010000 000188 00   A  0   0  1
    #     [ 2] .text             PROGBITS        08000190 010190 00490c 00  AX  0   0 16
    #     [ 3] .rodata           PROGBITS        08004aa0 014aa0 000328 00   A  0   0  8
    # Regex test: https://regex101.com/r/N3YQYw/1
    pattern_fields = [
        r'\s*',
        r'\[\s*', g('num', r'\d+'), r'\]',
        r'\s+',
        g('name', r'\S+'),
        r'\s+',
        g('type', r'\S+'),
        r'\s+',
        g('address', r'[0-9a-fA-F]+'),
        r'\s+',
        g('offset', r'[0-9a-fA-F]+'),
        r'\s+',
        g('size', r'[0-9a-fA-F]+'),
        r'\s+',
        g('entry_size', r'[0-9a-fA-F]+'),  # whatever it is we don't need it
        r'\s+',
        g('flags', r'\S*'),
        r'\s+',
        g('link', r'[0-9a-fA-F]+'),  # whatever it is we don't need it
        r'\s+',
        g('info', r'[0-9a-fA-F]+'),  # whatever it is we don't need it
        r'\s+',
        g('alignment', r'[0-9a-fA-F]+'),  # whatever it is we don't need it
        r'\s*'
    ]
    pattern = r'^{}$'.format(r''.join(pattern_fields))
    pattern = re.compile(pattern)

    class Flag:
        # key to flags
        WRITE = 'W'
        ALLOC = 'A'
        EXECUTE = 'X'
        MERGE = 'M'
        STRINGS = 'S'
        INFO = 'I'
        LINK_ORDER = 'L'
        EXTRA_OS_PROCESSINg_REQUIRED = 'O'
        GROUP = 'G'
        TLS = 'T'
        COMPRESSED = 'C'
        UNKNOWN = 'x'
        OS_SPECIFIC = 'o'
        EXCLUDE = 'E'
        PURECODE = 'y'
        PROCESSOR_SPECIFIC = 'p'

        @classmethod
        def to_string(cls, flag):
            for name, value in vars(cls).items():
                if not name.startswith('_'):
                    if value == flag:
                        return name
            return None

    def __init__(self, **kwargs):
        self.num = kwargs['num']
        self.name = kwargs['name']
        self.type = kwargs['type']
        self.address = kwargs['address']
        self.offset = kwargs['offset']
        self.size = kwargs['size']
        self.entry_size = kwargs['entry_size']
        self.flags = kwargs['flags']
        self.link = kwargs['link']
        self.info = kwargs['info']
        self.alignment = kwargs['alignment']

    def is_read_only(self):
        return self.Flag.WRITE not in self.flags

    def occupies_memory(self):
        # these are the only relevant sections for us
        return self.Flag.ALLOC in self.flags

    # these two methods are probably a big simplification
    # as they may be true only for small embedded systems
    def occupies_rom(self):
        return self.occupies_memory() and \
            self.type not in ['NOBITS']

    def occupies_ram(self):
        return self.occupies_memory() and not self.is_read_only()

    @classmethod
    def from_readelf_line(cls, line):
        """
        Create a Section from a line of `readelf -WS` output.
        """
        m = cls.pattern.match(line)
        if not m:
            log.debug('no match: ' + line.strip())
            return None

        # convert non-string values
        m = m.groupdict()
        m['num'] = int(m['num'])
        m['address'] = int(m['address'], 16)
        m['offset'] = int(m['offset'], 16)
        m['size'] = int(m['size'], 16)
        m['entry_size'] = int(m['entry_size'], 16)
        # not sure if these are base-16 or base-10
        m['link'] = int(m['link'], 10)
        m['info'] = int(m['info'], 10)
        m['alignment'] = int(m['alignment'], 10)

        return Section(**m)

    @classmethod
    def print(cls, sections):
        lines = []
        for s in sections:
            fields = [str(s.num), s.name, s.type,
                      hex(s.address), sizeof_fmt(s.size),
                      ','.join(cls.Flag.to_string(f) for f in s.flags)]
            lines.append(fields)
        sizes = [max(len(l[i]) for l in lines) for i in range(6)]
        h_fmt = '{:%d}   {:%d}   {:%d}   {:%d}   {:%d}   {:%d}' % (*sizes, )
        fmt = '{:>%d}   {:%d}   {:%d}   {:>%d}   {:>%d}   {:%d}' % (*sizes, )
        header = h_fmt.format('N', 'Name', 'Type', 'Addr', 'Size', 'Flags')
        separator = '=' * len(header)
        top_header = '{:=^{size}s}'.format(' SECTIONS ', size=len(separator))
        print(Color.BOLD + top_header  + Color.RESET)
        print(Color.BOLD + header + Color.RESET)
        print(Color.BOLD + separator  + Color.RESET)
        for line in lines:
            print(fmt.format(*line))
        print(Color.BOLD + separator  + Color.RESET)

    @classmethod
    def extract_sections_info(cls, elf_file, readelf_exe='readelf'):
        """
        Uses binutils 'readelf' to find info about all sections from an ELF file.
        """
        flags = ['--wide', '--section-headers']
        readelf_proc = subprocess.Popen([readelf_exe, *flags, elf_file],
                                        stdout=subprocess.PIPE, universal_newlines=True)

        # parse lines
        log.info('Using readelf sections regex: %s' % cls.pattern.pattern)
        sections = [Section.from_readelf_line(l) for l in readelf_proc.stdout]
        sections = list(filter(None, sections))

        if readelf_proc.wait(3) != 0:
            raise subprocess.CalledProcessError(readelf_proc.returncode,
                                                readelf_proc.args)

        return sections


class SymbolsTreeByPath:
    """A tree built from symbols grouped by paths. Nodes can be symbols or paths."""

    class Node(TreeNode):
        def __init__(self, data, is_dir=False, *args, **kwargs):
            self.data = data
            self._is_dir = is_dir
            self.cumulative_size = None  # used for accumulating symbol sizes in paths
            super().__init__(*args, **kwargs)

        def is_symbol(self):
            return isinstance(self.data, Symbol)

        def is_root(self):
            return self.data is None

        def is_path(self):
            return not self.is_root() and not self.is_symbol()

        def is_dir(self):
            return self.is_path() and self._is_dir

        def is_file(self):
            return self.is_path() and not self._is_dir

        def __repr__(self):
            string = self.data.name if self.is_symbol() else self.data
            return 'Node(%s)' % string

    def __init__(self, symbols=[]):
        self.tree_root = self.Node(None)
        self.orphans = self.Node('?')
        self.tree_root.add(self.orphans)
        for symbol in symbols:
            self.add(symbol)
        self.total_size = None

    def add(self, symbol):
        assert isinstance(symbol, Symbol), "Only instances of Symbol can be added!"
        if symbol.file is None:
            self.orphans.add(self.Node(symbol))
        else:
            if not os.path.isabs(symbol.file):
                log.warning('Symbol\'s path is not absolute: %s: %s'
                                % (symbol, symbol.file))
            self._add_symbol_with_path(symbol)

    def _add_symbol_with_path(self, symbol):
        """
        Adds the given symbol by creating nodes for each path component
        before adding symbol as the last ("leaf") node.
        """
        path = pathlib.Path(symbol.file)
        node = self.tree_root
        for part in path.parts:
            # find it the part exists in children
            path_children = filter(self.Node.is_path, node.children)
            path_child = list(filter(lambda node: node.data == part, path_children))
            assert len(path_child) <= 1
            # if it does not exsits, then create it and add
            if len(path_child) == 0:
                path_child = self.Node(part, is_dir=True)
                node.add(path_child)
            else:
                path_child = path_child[0]
            # go 'into' this path part's node
            node = path_child
        # remove directory signature from last path part
        node._is_dir = False
        # last, add the symbol, the "tree leaf"
        node.add(self.Node(symbol))

    def merge_paths(self, fish_like=False):
        """Merges all path componenets that have only one child into single nodes."""
        for node, depth in self.tree_root.pre_order():
            # we want only path nodes that have only one path node
            if node.is_path() and len(node.children) == 1:
                child = node.children[0]
                if child.is_path():
                    # add this node's path to its child
                    this_path = node.data
                    if fish_like:
                        head, tail = os.path.split(this_path)
                        this_path = os.path.join(head, tail[:1])
                    child.data = os.path.join(this_path, child.data)
                    # remove this node and reparent its child
                    node.parent.children.remove(node)
                    node.parent.add(child)

    def sort(self, key, reverse=False):
        """
        Sort all symbol lists by the given key - function that takes a Symbol as an argument.
        sort_paths_by_name - if specified, then paths are sorted by name (directories first).
        reverse - applies to symbols
        reverse_paths - appliesto paths (still, directories go first)
        """
        # to avoid sorting the same list many times, gather them first
        nodes_with_children = []
        for node, depth in self.tree_root.pre_order():
            if len(node.children) > 1:
                nodes_with_children.append(node)
        for node in nodes_with_children:
            # we need tee to split generators into many so that filter will work as expected
            ch1, ch2 = itertools.tee(node.children)
            symbols = filter(self.Node.is_symbol, ch1)
            non_symbols = filter(lambda n: not n.is_symbol(), ch2)
            # sort others by size if available else by name, directories first
            # add - to size, as we need reverse sorting for path names
            path_key = lambda node: -node.cumulative_size if node.cumulative_size is not None else node.data
            ns1, ns2, ns3 = itertools.tee(non_symbols, 3)
            dirs = filter(self.Node.is_dir, ns1)
            files = filter(self.Node.is_file, ns2)
            others = filter(lambda n: not n.is_file() and not n.is_dir(), ns3)
            non_symbols = sorted(dirs, key=path_key) \
                + sorted(files, key=path_key) + list(others)
            symbols = sorted(symbols, key=lambda node: key(node.data), reverse=reverse)
            children = list(non_symbols) + list(symbols)
            node.children = children

    def accumulate_sizes(self, reset=True):
        """
        Traverse tree bottom-up to accumulate symbol sizes in paths.
        """
        if reset:
            for node, depth in self.tree_root.pre_order():
                node.cumulative_size = None
        for node, depth in self.tree_root.post_order():
            if node.parent is None:
                continue
            if node.parent.cumulative_size is None:
                node.parent.cumulative_size = 0
            if node.is_symbol():
                node.cumulative_size = node.data.size
            node.parent.cumulative_size += node.cumulative_size

    def calculate_total_size(self):
        # calculate the total size
        all_nodes = (node for node, _ in self.tree_root.pre_order())
        all_symbols = filter(self.Node.is_symbol, all_nodes)
        self.total_size = sum(s.data.size for s in all_symbols)

    class Protoline:
        def __init__(self, depth=0, node=None, string=None, colors=None):
            self.depth = depth
            self.node = node
            self.string = string
            self.field_strings = []
            self.colors = colors or []  # avoid creating one list shared by all objects

        def print(self):
            if len(self.colors) > 0:
                print(sum(self.colors, Color()) + self.string + Color.RESET)
            else:
                print(self.string)

    def generate_printable_lines(self, *, max_width=80, min_size=0, header=None, indent=2,
                                 colors=True, alternating_colors=False, trim=True, human_readable=False):
        """
        Creates printable output in form of Protoline objects.
        Handles RIDICULLOUSLY complex printing. Someone could probably implement it easier.
        """
        # create and initially fill the lines
        protolines = self._generate_protolines(min_size)
        self._add_field_strings(protolines, indent, human_readable)
        # formatting string
        h_fmt = '{:{s0}}   {:{s1}}   {:{s2}}'
        fmt = '{:{s0}}   {:>{s1}}   {:>{s2}}'
        t_fmt = '{:{s0}}   {:>{s1}}   {:>{s2}}'
        table_headers = ('Symbol', 'Size', '%')
        # calculate sizes
        field_sizes = self._calculate_field_sizes(protolines, max_width=max_width,
                                                  initial=[len(h) for h in table_headers])
        # trim too long strings
        if trim:
            self._trim_strings(protolines, field_sizes)
        # prepare sizes dict
        sizes_dict = {'s%d' % i: s for i, s in enumerate(field_sizes)}
        # "render" the strings
        for line in protolines:
            if line.string is None:
                if len(line.field_strings) == 0:
                    line.string = ''
                else:
                    line.string = fmt.format(*line.field_strings, **sizes_dict)
        # preopare table header
        header_lines = self._create_header_protolines(h_fmt, table_headers, sizes_dict, header)
        for l in reversed(header_lines):
            protolines.insert(0, l)
        # prepare totals
        if self.total_size is not None:
            totals_lines = self._create_totals_protolines(t_fmt, sizes_dict, human_readable)
            protolines.extend(totals_lines)
        # add colors
        if colors:
            self._add_colors(protolines, alternating_colors)
        return protolines

    def _generate_protolines(self, min_size):
        # generate list of nodes with indent to be printed
        protolines = []
        for node, depth in self.tree_root.pre_order():
            # we never print root so subtract its depth
            depth = depth - 1
            if node.is_root():
                continue
            elif not (node.is_symbol() or node.is_path()):
                raise Exception('Wrong symbol type encountered')
            elif node.is_symbol() and node.data.size < min_size:
                continue
            protolines.append(self.Protoline(depth, node))
        return protolines

    def _add_field_strings(self, protolines, indent, human_readable):
        for line in protolines:
            indent_str = ' ' * indent * line.depth
            if line.node.is_path():
                size_str, percent_str = '-', '-'
                if line.node.cumulative_size is not None:
                    size_str = self._size_string(line.node.cumulative_size, human_readable)
                    if self.total_size is not None:
                        percent_str = '%.2f' % (line.node.cumulative_size / self.total_size * 100)
                fields = [indent_str + line.node.data, size_str, percent_str]
            elif line.node.is_symbol():
                percent_str = '-'
                if self.total_size is not None:
                    percent_str = '%.2f' % (line.node.data.size / self.total_size * 100)
                size_str = self._size_string(line.node.data.size, human_readable)
                fields = [indent_str + line.node.data.name, size_str, percent_str]
            else:
                raise Exception('Wrong symbol type encountered')
            line.field_strings = fields

    def _calculate_field_sizes(self, protolines, initial, max_width=0):
        field_sizes = initial
        for line in protolines:
            for i, s, in enumerate(line.field_strings):
                field_sizes[i] = max(len(s), field_sizes[i])
        # trim the fields if max_width is > 0
        if max_width > 0:
            if sum(field_sizes) > max_width:
                field_sizes[0] -= sum(field_sizes) - max_width
        return field_sizes

    def _trim_strings(self, protolines, field_sizes):
        for line in protolines:
            for i, s, in enumerate(line.field_strings):
                if len(s) > field_sizes[i]:
                    line.field_strings[i] = s[:field_sizes[i] - 3] + '...'

    def _create_header_protolines(self, header_fmt, table_headers, sizes_dict, header):
        table_header = header_fmt.format(*table_headers, **sizes_dict)
        separator = self._separator_string(len(table_header))
        if header is None:
            header = separator
        else:
            h = ' %s ' % header
            mid = len(separator) // 2
            before, after = int(math.ceil(len(h)/2)), int(math.floor(len(h)/2))
            header = separator[:mid - before] + h + separator[mid+after:]
        header_protolines = [self.Protoline(string=s) for s in [header, table_header, separator]]
        return header_protolines

    def _create_totals_protolines(self, fmt, sizes_dict, human_readable):
        totals = fmt.format('Symbols total', self._size_string(self.total_size, human_readable), '',
                            **sizes_dict)
        separator = self._separator_string(len(totals))
        return [self.Protoline(string=s) for s in [separator, totals, separator]]

    def _separator_string(self, length):
        return '=' * length

    def _add_colors(self, protolines, alternating_colors):
        second_symbol_color = False
        for line in protolines:
            c = []
            if line.node is None:  # header lines
                c = [Color.BOLD, Color.BLUE]
            elif line.node.is_file():
                c = [Color.L_BLUE]
            elif line.node.is_dir():
                c = [Color.BLUE]
            elif line.node.is_symbol():
                if second_symbol_color and alternating_colors:
                    c = [Color.L_GREEN]
                    second_symbol_color = False
                else:
                    c = [Color.L_YELLOW]
                    second_symbol_color = True
            line.colors += c

    def _size_string(self, size, human_readable):
        if human_readable:
            return sizeof_fmt(size)
        return str(size)


################################################################################

def main():
    result = False
    args = parse_args()

    # adjust verbosity
    if args.verbose:
        level = log.level - 10 * args.verbose
        log.setLevel(max(level, logging.DEBUG))

    # prepare arguments
    if not os.path.isfile(args.elf):
        print('ELF file %s does not exist' % args.elf, file=sys.stderr)
        return result

    if not any([args.rom, args.ram, args.print_sections, args.use_sections]):
        print('No memory type action specified (RAM/ROM or special). See -h for help.')
        return result

    def get_exe(name):
        cmd = args.toolchain_triplet + name
        if 'Windows' == platform.system():
            cmd = cmd + '.exe'
        assert shutil.which(cmd) is not None, \
            'Executable "%s" could not be found!' % cmd
        return args.toolchain_triplet + name

    # process symbols
    symbols = Symbol.extract_elf_symbols_info(args.elf, get_exe('readelf'))
    fileinfo = extract_elf_symbols_fileinfo(args.elf, get_exe('nm'))
    add_fileinfo_to_symbols(fileinfo, symbols)

    # demangle only after fileinfo extraction!
    if not args.no_demangle:
        demangle_symbol_names(symbols, get_exe('c++filt'))

    # load section info
    sections = Section.extract_sections_info(args.elf, get_exe('readelf'))
    sections_dict = {sec.num: sec for sec in sections}

    def print_tree(header, symbols):
        tree = SymbolsTreeByPath(symbols)
        if not args.no_merge_paths:
            tree.merge_paths(args.fish_paths)
        if not args.no_cumulative_size:
            tree.accumulate_sizes()
        if args.sort_by_name:
            tree.sort(key=lambda symbol: symbol.name, reverse=False)
        else:  # sort by size
            tree.sort(key=lambda symbol: symbol.size, reverse=True)
        if not args.no_totals:
            tree.calculate_total_size()
        min_size = math.inf if args.files_only else args.min_size
        lines = tree.generate_printable_lines(
            header=header, colors=not args.no_color, human_readable=args.human_readable,
            max_width=args.max_width, min_size=min_size, alternating_colors=args.alternating_colors)
        for line in lines:
            line.print()

    def filter_symbols(section_key):
        secs = filter(section_key, sections)
        secs_str = ', '.join(s.name for s in secs)
        log.info('Considering sections: ' + secs_str)
        filtered = filter(lambda symbol: section_key(sections_dict.get(symbol.section, None)),
                          symbols)
        out, test = itertools.tee(filtered)
        if len(list(test)) == 0:
            print("""
ERROR: No symbols from given section found or all were ignored!
       Sections were: %s
            """.strip() % secs_str, file=sys.stderr)
            sys.exit(1)
        return out

    if args.print_sections:
        Section.print(sections)

    if args.rom:
        print_tree('ROM', filter_symbols(lambda sec: sec and sec.occupies_rom()))

    if args.ram:
        print_tree('RAM', filter_symbols(lambda sec: sec and sec.occupies_ram()))

    if args.use_sections:
        nums = list(map(int, args.use_sections))
        #  secs = list(filter(lambda s: s.num in nums, sections))
        name = 'SECTIONS: %s' % ','.join(map(str, nums))
        print_tree(name, filter_symbols(lambda sec: sec and sec.num in nums))

    return True


if __name__ == "__main__":
    result = main()
    if not result:
        sys.exit(1)

