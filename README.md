gumbo-node
==========

Node bindings for Google's gumbo html parser


Goals
-----

- provide a direct port of the Gumbo API for Node


API
---

Node
- type: Number
- parent: Node
- indexWithinParent: Number
- parseFlags: String

Document:
- children: Array of Nodes
- hasDoctype: Boolean
- name: String
- publicIdentifier: String
- systemIdentifier: String
- docTypeQuirksMode: String

Element:
- children: Array of Nodes
- tag: gumbo normalized tag name
- tagNamespace: String
- originalTag: tag from source
- originalEndTag: String
- attributes: hash of Attributes
- startPos: Position
- endPos: Position

Attribute:
- attrNamespace: String
- name: String
- originalName: String
- value: String
- originalValue: String
- nameStart: Position
- nameEnd: Position
- valueStart: Position
- valueEnd: Position

Position:
- line: line number (1-indexed)
- column: column number (1-indexed)
- offset: byte offset (0-indexed)

Text/Comment/CDATA:
- text: String
- originalText: String
- starPos: Position


Thanks
------

- Much of this work was based on [karlwestin/gumbo-parser](http://github.com/karlwestin/gumbo-parser) .gyp bindings
- authors of Gumbo for making such a convenient library
