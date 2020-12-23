The initial layout of the pages is defined by creating a special file on the SPIFFS file system.
This layout is displayed each time HASP starts up.
You can upload this file *(and other resource assets like fonts and images)* in the web interface Configuration > HASP Settings.

### pages.jsonl

The location of this file is `/pages.jsonl` in the root of the filesystem.
It uses the [JSON Lines format](http://www.jsonlines.org) with one json object per line.
Each line should contain exactly **one** valid json object and end with a line-break `\n` *(not a comma)*.

The file is interpreted **line-by-line**.
When a malformed line is encountered, the processing of the rest of the file stops.
If you are missing objects, check the logs to see which line was processed last.
You probably have a typo in the following line which blocks parsing the rest of the file.

?> The complete file in its entirety is *not* a valid json file.
?> Each individual line however must be a valid json object.
?> The file extension is `.jsonl` and not `.json`.

### Objects
Each line in `pages.jsonl` creates **one object** on a page and has to be in the json format.
The order of objects dictates the layer on the page from bottom to top.

Example Objects
```json
{"page":2,"id":1,"objid":12,"x":5,"y":20,"h":50,"w":50,"enabled":"true","hidden":"false"}
{"page":2,"id":2,"objid":50,"x":5,"y":120,"h":90,"w":50,"enabled":"false","hidden":"false"}
```

Once the object is created, you can reference it with `p[x].b[y]` where `x` is the pagenumber and `y` is the id of the object.

for example:
```
p[2].b[1].w=100
p[2].b[2].hidden=true
```

## Comments
If any of the required `id` or `objid` properties are missing -*and the line is still valid json*- then it is interpreted as a comment.

You can also use the `page` parameter in a comment to set the default page for new objects that don't have a `page` parameter.

Example 1: Add a comment on a single line that is ignored.
```json
{"comment":" ----------- Page 2 layout ------------"}
```

Example 2: Set the default `page` for next object(s) to `3` besides adding a comment as well.

```json
{"page":3,"comment":" ---- My Awesome Color Picker Layout ----"}
```

If you then omit the `page` parameter in the lines below this comment, those objects will appear by default on page `3`.

?> If the line is not valid json, the parsing of the rest of the file is also stopped.

## Blank Lines
Blank lines are allowed for readability and are ignored.
