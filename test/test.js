var gumbo = require('../build/Release/gumbo');
var fs = require('fs');
var assert = require('assert');


function testParse(err, text) {
    if (err) {
        throw err;
    }

    console.log("Running tests");

    var tree = gumbo.parse(text);
    assert(!!tree, "Returned a value");

    assert(tree.tagName == 'html', "Root node is <html>");
    assert(tree.children[0].tagName == 'head');
    assert(tree.children[0].children[1].tagName == 'title');
    assert(tree.children[2].tagName == 'body');
    assert(tree.children[2].children[1].nodeName == '#comment');
    assert(tree.children[2].children[3].attributes['class'] == 'waffle');
}


fs.readFile(__dirname + '/test.html', {encoding: 'utf-8'}, testParse);
