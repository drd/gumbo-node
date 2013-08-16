#include <node.h>
#include <v8.h>

#include "deps/gumbo-parser/src/gumbo.h"


#ifdef __DEBUG__
#define DPRINTF(fmt, args...) do { printf(fmt "\n", ##ARGS); } while(0);
#else
#define DPRINTF(fmt, args...) do {} while(0);
#endif


using namespace v8;


Local<Object> create_parse_tree(GumboNode* root);
Local<Object> consume_element(GumboElement* element);
Local<Object> consume_text(GumboText* text);
Local<Object> consume_comment(GumboText* text);
Local<Object> consume_cdata(GumboText* text);


void record_location(Local<Object> node, GumboSourcePosition* pos) {
    Local<Object> position = Object::New();
    position->Set(String::NewSymbol("line"),
		  Number::New(pos->line));
    position->Set(String::NewSymbol("column"),
		  Number::New(pos->column));
    position->Set(String::NewSymbol("offset"),
		  Number::New(pos->offset));
    node->Set(String::NewSymbol("position"), position);
}


Local<Object> consume_element(GumboElement* element) {
    Local<Object> element_node = Object::New();
    element_node->Set(String::NewSymbol("tagName"),
		      String::New(gumbo_normalized_tagname(element->tag)));

    // TODO: omit brackets and attr list
    element_node->Set(String::NewSymbol("originalTag"),
		      String::New(element->original_tag.data,
				  element->original_tag.length));
    element_node->Set(String::NewSymbol("nodeType"), Number::New(1));

    Local<Object> attributes = Object::New();
    element_node->Set(String::NewSymbol("attributes"), attributes);

    GumboVector* element_attrs = &element->attributes;
    for (uint i=0; i < element_attrs->length; i++) {
	GumboAttribute* element_attr = (GumboAttribute* )element_attrs->data[i];
	attributes->Set(String::New(element_attr->name),
			String::New(element_attr->value));
    }

    Local<Array> children = Array::New();
    element_node->Set(String::NewSymbol("children"),
	      children);

    GumboVector* element_children = &element->children;
    for (uint i=0; i < element->children.length; i++) {
	GumboNode* element_child = (GumboNode* )element_children->data[i];
	Local<Object> child = create_parse_tree(element_child);
	children->Set(Number::New(i), child);
    }

    record_location(element_node, &element->start_pos);
    return element_node;
}


Local<Object> consume_text(GumboText* text) {
    Local<Object> text_node = Object::New();
    text_node->Set(String::NewSymbol("nodeName"), String::NewSymbol("#text"));
    text_node->Set(String::NewSymbol("text"),
		   String::New(text->text));
    text_node->Set(String::NewSymbol("nodeType"), Number::New(3));

    record_location(text_node, &text->start_pos);
    return text_node;
}


Local<Object> consume_comment(GumboText* text) {
    Local<Object> comment_node = Object::New();
    comment_node->Set(String::NewSymbol("nodeName"), String::NewSymbol("#comment"));
    comment_node->Set(String::NewSymbol("text"),
		      String::New(text->text));
    comment_node->Set(String::NewSymbol("nodeType"), Number::New(8));

    record_location(comment_node, &text->start_pos);
    return comment_node;
}


Local<Object> consume_cdata(GumboText* text) {
    Local<Object> cdata_node = Object::New();
    cdata_node->Set(String::NewSymbol("nodeName"), String::NewSymbol("#comment"));
    cdata_node->Set(String::NewSymbol("text"),
		    String::New(text->text));
    cdata_node->Set(String::NewSymbol("nodeType"), Number::New(4));

    record_location(cdata_node, &text->start_pos);
    return cdata_node;
}


Local<Object> create_parse_tree(GumboNode* node) {
    Local<Object> parsed;

    switch (node->type) {
    case GUMBO_NODE_ELEMENT:
	parsed = consume_element(&node->v.element);
	break;
    case GUMBO_NODE_WHITESPACE:
    case GUMBO_NODE_TEXT:
	parsed = consume_text(&node->v.text);
	break;
    case GUMBO_NODE_CDATA:
	parsed = consume_cdata(&node->v.text);
	break;
    case GUMBO_NODE_COMMENT:
	parsed = consume_comment(&node->v.text);
	break;
    case GUMBO_NODE_DOCUMENT:
	// should never happen
	break;
    }

    return parsed;
}


Handle<Value> Method(const Arguments& args) {
    HandleScope scope;

    if (args.Length() != 1) {
	ThrowException(Exception::TypeError(String::New("Please give Gumbo an HTML string")));
	return scope.Close(Undefined());
    }

    if (!args[0]->IsString()) {
	ThrowException(Exception::TypeError(String::New("Gumbo works on an HTML string")));
	return scope.Close(Undefined());
    }

    String::Utf8Value str(args[0]->ToString());
    char* c_str = *str;

    GumboOutput* output = gumbo_parse_with_options(&kGumboDefaultOptions,
						   c_str,
						   args[0]->ToString()->Utf8Length());

    Local<Object> tree = create_parse_tree(output->root);

    gumbo_destroy_output(&kGumboDefaultOptions, output);

    return scope.Close(tree);
}


void init(Handle<Object> exports) {
    exports->Set(String::NewSymbol("parse"),
		 FunctionTemplate::New(Method)->GetFunction());
}


NODE_MODULE(gumbo, init)
