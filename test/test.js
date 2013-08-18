var gumbo = require('../build/Release/gumbo');
var fs = require('fs');
var assert = require('assert');


function testParse(err, text) {
    if (err) {
        throw err;
    }

    console.log("Running tests");

    var document = gumbo.parse(text);
    assert(!!document, "Returned a value");

    var tree = document.children[0];
    // TODO: replace this with real tests :)
    assert(tree.tag == 'html', "Root node is <html>");
    assert(tree.children[0].tag == 'head');
    assert(tree.children[0].children[1].tag == 'title');
    assert(tree.children[2].tag == 'body');
    assert(tree.children[2].children[1].text == ' hark, a comment! ');
    assert(tree.children[2].children[3].attributes['class'].value == 'waffle');
}


fs.readFile(__dirname + '/test.html', {encoding: 'utf-8'}, testParse);
