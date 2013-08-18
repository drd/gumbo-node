#include <node.h>
#include <v8.h>

#include "deps/gumbo-parser/src/gumbo.h"


#ifdef __DEBUG__
#define DPRINTF(fmt, args...) do { printf(fmt "\n", ##ARGS); } while(0);
#else
#define DPRINTF(fmt, args...) do {} while(0);
#endif


using namespace v8;


Handle<Value> create_parse_tree(GumboNode* root, Handle<Value> parent);
Local<Array> get_children(GumboVector* children);
Local<Object> consume_document(GumboDocument* document);
Local<Object> consume_element(GumboElement* element, Handle<Value> parent);
Local<Object> consume_text(GumboText* text);


void record_location(Local<Object> node, GumboSourcePosition* pos, const char* name) {
    Local<Object> position = Object::New();
    position->Set(String::NewSymbol("line"),
		  Number::New(pos->line));
    position->Set(String::NewSymbol("column"),
		  Number::New(pos->column));
    position->Set(String::NewSymbol("offset"),
		  Number::New(pos->offset));
    node->Set(String::NewSymbol(name), position);
}


Local<Array> get_children(GumboVector* children, Handle<Value> parent) {
    Local<Array> node_children = Array::New();

    for (uint i=0; i < children->length; i++) {
	GumboNode* node_child = (GumboNode* )children->data[i];
	Handle<Value> child = create_parse_tree(node_child, parent);
	node_children->Set(Number::New(i), child);
    }

    return node_children;
}


Handle<Value> get_quirks_mode(GumboQuirksModeEnum mode) {
    const char* mode_name;
    switch (mode) {
    case GUMBO_DOCTYPE_NO_QUIRKS:
	mode_name = "noQuirks";
	break;
    case GUMBO_DOCTYPE_QUIRKS:
	mode_name = "quirks";
	break;
    case GUMBO_DOCTYPE_LIMITED_QUIRKS:
	mode_name = "limitedQuirks";
	break;
    default:
	// TODO: raise exception?
	ThrowException(
            Exception::TypeError(String::New("Unknown QuirksMode type")));
	return Undefined();
    }

    return String::NewSymbol(mode_name);
}


Local<Object> consume_document(GumboDocument* document) {
    Local<Object> document_node = Object::New();
    document_node->Set(String::NewSymbol("hasDoctype"),
		       Boolean::New(document->has_doctype));

    document_node->Set(String::NewSymbol("name"),
		       String::New(document->name));
    document_node->Set(String::NewSymbol("publicIdentifier"),
		       String::New(document->public_identifier));
    document_node->Set(String::NewSymbol("children"),
		       String::New(document->system_identifier));

    document_node->Set(String::NewSymbol("docTypeQuirksMode"),
		       get_quirks_mode(document->doc_type_quirks_mode));

    document_node->Set(String::NewSymbol("children"),
		       get_children(&document->children, document_node));

    return document_node;
}


Handle<Value>
get_attribute_namespace(GumboAttributeNamespaceEnum attr_namespace) {
    const char* namespace_name;

    switch (attr_namespace) {
    case GUMBO_ATTR_NAMESPACE_XLINK:
	namespace_name = "xlink";
	break;
    case GUMBO_ATTR_NAMESPACE_XML:
	namespace_name = "xml";
	break;
    case GUMBO_ATTR_NAMESPACE_XMLNS:
	namespace_name = "xmlns";
	break;
    case GUMBO_ATTR_NAMESPACE_NONE:
	return Null();
	break;
    default:
	ThrowException(
            Exception::TypeError(String::New("Unknown attribute namespace")));
	return Undefined();
	break;
    }

    return String::NewSymbol(namespace_name);
}


Local<Object> get_attribute(GumboAttribute* attr) {
    Local<Object> attribute = Object::New();

    attribute->Set(String::NewSymbol("namespace"),
		   get_attribute_namespace(attr->attr_namespace));

    attribute->Set(String::NewSymbol("name"),
		   String::New(attr->name));
    attribute->Set(String::NewSymbol("originalName"),
		   String::New(attr->original_name.data,
			       attr->original_name.length));

    attribute->Set(String::NewSymbol("value"),
		   String::New(attr->value));
    attribute->Set(String::NewSymbol("originalValue"),
		   String::New(attr->original_value.data,
			       attr->original_value.length));

    record_location(attribute, &attr->name_start, "nameStart");
    record_location(attribute, &attr->name_end, "nameEnd");

    record_location(attribute, &attr->value_start, "valueStart");
    record_location(attribute, &attr->value_end, "valueEnd");

    return attribute;
}

Local<Object> get_attributes(GumboVector* element_attrs) {
    Local<Object> attributes = Object::New();

    for (uint i=0; i < element_attrs->length; i++) {
	GumboAttribute* element_attr = (GumboAttribute* )element_attrs->data[i];
	attributes->Set(String::New(element_attr->name),
			get_attribute(element_attr));
    }

    return attributes;
}


Handle<Value> get_tag_namespace(GumboNamespaceEnum tag_namespace) {
    const char* namespace_name;

    switch (tag_namespace) {
    case GUMBO_NAMESPACE_HTML:
	namespace_name = "HTML";
	break;
    case GUMBO_NAMESPACE_SVG:
	namespace_name = "SVG";
	break;
    case GUMBO_NAMESPACE_MATHML:
	namespace_name = "MATHML";
	break;
    default:
	ThrowException(
            Exception::TypeError(String::New("Unknown tag namespace")));
	return Undefined();
    }

    return String::NewSymbol(namespace_name);
}


Local<Object> consume_element(GumboElement* element, Handle<Value> parent) {
    Local<Object> element_node = Object::New();
    element_node->Set(String::NewSymbol("tag"),
		      String::New(gumbo_normalized_tagname(element->tag)));

    element_node->Set(String::NewSymbol("tagNamespace"),
		      get_tag_namespace(element->tag_namespace));

    // TODO: omit brackets and attr list
    element_node->Set(String::NewSymbol("originalTag"),
		      String::New(element->original_tag.data,
				  element->original_tag.length));

    element_node->Set(String::NewSymbol("originalEndTag"),
		      String::New(element->original_end_tag.data,
				  element->original_end_tag.length));

    Local<Object> attributes = Object::New();
    element_node->Set(String::NewSymbol("attributes"),
		      get_attributes(&element->attributes));


    element_node->Set(String::NewSymbol("children"),
		      get_children(&element->children, element_node));

    record_location(element_node, &element->start_pos, "startPos");
    record_location(element_node, &element->end_pos, "endPos");
    return element_node;
}


Local<Object> consume_text(GumboText* text) {
    Local<Object> text_node = Object::New();
    text_node->Set(String::NewSymbol("text"),
		   String::New(text->text));
    text_node->Set(String::NewSymbol("originalText"),
		   String::New(text->original_text.data,
			       text->original_text.length));

    record_location(text_node, &text->start_pos, "startPos");
    return text_node;
}


Handle<Value> get_parse_flags(GumboParseFlags flags) {
    const char* flag_name;

    switch(flags) {
    case GUMBO_INSERTION_NORMAL:
	flag_name = "normal";
	break;
    case GUMBO_INSERTION_BY_PARSER:
	flag_name = "byParser";
	break;
    case GUMBO_INSERTION_IMPLICIT_END_TAG:
	flag_name = "implicitEndTag";
	break;
    case GUMBO_INSERTION_IMPLIED:
	flag_name = "implied";
	break;
    case GUMBO_INSERTION_CONVERTED_FROM_END_TAG:
	flag_name = "convertedFromEndTag";
	break;
    case GUMBO_INSERTION_FROM_ISINDEX:
	flag_name = "fromIsindex";
	break;
    case GUMBO_INSERTION_FROM_IMAGE:
	flag_name = "fromImage";
	break;
    case GUMBO_INSERTION_RECONSTRUCTED_FORMATTING_ELEMENT:
	flag_name = "reconstructedFormattingElement";
	break;
    case GUMBO_INSERTION_ADOPTION_AGENCY_CLONED:
	flag_name = "adoptionAgencyCloned";
	break;
    case GUMBO_INSERTION_ADOPTION_AGENCY_MOVED:
	flag_name = "adoptionAgencyMoved";
	break;
    case GUMBO_INSERTION_FOSTER_PARENTED:
	flag_name = "fosterParented";
	break;
    default:
	ThrowException(Exception::TypeError(String::New("Unknown parse flag")));
	return Undefined();
    }

    return String::NewSymbol(flag_name);
}


Handle<Value> get_node_type(GumboNodeType node_type) {
    const char* type_name;

    switch (node_type) {
    case GUMBO_NODE_DOCUMENT:
	type_name = "document";
	break;
    case GUMBO_NODE_ELEMENT:
	type_name = "element";
	break;
    case GUMBO_NODE_TEXT:
	type_name = "text";
	break;
    case GUMBO_NODE_CDATA:
	type_name = "cdata";
	break;
    case GUMBO_NODE_COMMENT:
	type_name = "comment";
	break;
    case GUMBO_NODE_WHITESPACE:
	type_name = "whitespace";
	break;
    default:
	ThrowException(Exception::TypeError(String::New("Unknown node type")));
	return Undefined();
    }

    return String::NewSymbol(type_name);
}


Handle<Value> create_parse_tree(GumboNode* node, Handle<Value> parent) {
    Local<Object> parsed;

    switch (node->type) {
    case GUMBO_NODE_ELEMENT:
	parsed = consume_element(&node->v.element, parent);
	break;

    case GUMBO_NODE_WHITESPACE:
    case GUMBO_NODE_TEXT:
    case GUMBO_NODE_CDATA:
    case GUMBO_NODE_COMMENT:
	parsed = consume_text(&node->v.text);
	break;

    case GUMBO_NODE_DOCUMENT:
	parsed = consume_document(&node->v.document);
	break;

    default:
	ThrowException(Exception::TypeError(String::New("Unknown node type")));
	return Undefined();
    }

    parsed->Set(String::NewSymbol("type"), get_node_type(node->type));

    parsed->Set(String::NewSymbol("parent"), parent);

    parsed->Set(String::NewSymbol("indexWithinParent"),
		Number::New(node->index_within_parent));

    parsed->Set(String::NewSymbol("parseFlags"),
		get_parse_flags(node->parse_flags));

    return parsed;
}


Handle<Value> Method(const Arguments& args) {
    HandleScope scope;

    if (args.Length() != 1) {
	ThrowException(Exception::TypeError
		       (String::New("Please give Gumbo a single HTML string")));
	return scope.Close(Undefined());
    }

    if (!args[0]->IsString()) {
	ThrowException(Exception::TypeError
		       (String::New("Gumbo works on an HTML string")));
	return scope.Close(Undefined());
    }

    String::Utf8Value str(args[0]->ToString());
    char* c_str = *str;

    GumboOutput* output = gumbo_parse_with_options(
			      &kGumboDefaultOptions,
			      c_str,
			      args[0]->ToString()->Utf8Length());

    Handle<Value> tree = create_parse_tree(output->document, Null());

    gumbo_destroy_output(&kGumboDefaultOptions, output);

    return scope.Close(tree);
}


void init(Handle<Object> exports) {
    exports->Set(String::NewSymbol("parse"),
		 FunctionTemplate::New(Method)->GetFunction());
}


NODE_MODULE(gumbo, init)
