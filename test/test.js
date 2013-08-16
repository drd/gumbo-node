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

    assert(tree.tag == 'html', "Root node is <html>");
}


fs.readFile(__dirname + '/test.html', {encoding: 'utf-8'}, testParse);
