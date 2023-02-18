import gzip, minify_html

# options = {
#     "do_not_minify_doctype": True,
#     "ensure_spec_compliant_unquoted_attribute_values": False,
#     "keep_closing_tags": True,
#     "keep_html_and_head_opening_tags": True,
#     "keep_spaces_between_attributes": True,
#     "keep_comments": True,
#     "minify_css": True,
#     "minify_js": True,
#     "remove_bangs": False,
#     "remove_processing_instructions": False
# }

# with open("data/script.js", "r") as f:
#     html=f.read()
# minified = minify_html.minify(html, options)
# with gzip.open('data/static/script.js.gz', 'wb') as f:
#   f.write(minified.encode('utf-8'))

# with open("data/editor.htm", "r") as f:
#     html=f.read() 
# # minified = minify_html.minify(html, options)
# with gzip.open('data/static/edit.htm.gz', 'wb') as f:
#   f.write(html.encode('utf-8'))
# with open('data/edit.htm', 'wb') as f:
#   f.write(html.encode('utf-8'))

  
# with open("data/style.css", "r") as f:
#     html=f.read() 
# with gzip.open('data/static/style.css.gz', 'wb') as f:
#   f.write(html.encode('utf-8'))
# minified = minify_html.minify(html, options).replace('"id=mdi-','" id=mdi-')
# with open('data/style.min.css', 'wb') as f:
#   f.write(minified.encode('utf-8'))

  
# with open("data/hasp.htm", "r") as f:
#     html=f.read() 
# minified = minify_html.minify(html, options)
# with gzip.open('data/static/hasp.htm.gz', 'wb') as f:
#   f.write(minified.encode('utf-8'))

  
  
# with open("data/en.json", "r") as f:
#     html=f.read() 
# minified = minify_html.minify(html, options)
# with gzip.open('data/static/en.json.gz', 'wb') as f:
#   f.write(minified.encode('utf-8'))

# with open("data/nl.json", "r") as f:
#     html=f.read() 
# minified = minify_html.minify(html, options)
# with gzip.open('data/static/nl.json.gz', 'wb') as f:
#   f.write(minified.encode('utf-8'))

# with open("data/fr.json", "r") as f:
#     html=f.read() 
# minified = minify_html.minify(html, options)
# with gzip.open('data/static/fr.json.gz', 'wb') as f:
#   f.write(minified.encode('utf-8'))

# with open("data/main.js", "r") as f:
#     html=f.read()
# with gzip.open('data/static/main.js.gz', 'wb') as f:
#   f.write(html.encode('utf-8'))
