#include "third_party/blink/renderer/core/html/html_script_element.h"

#include "third_party/blink/public/mojom/script/script_type.mojom-blink.h"
#include "third_party/blink/renderer/bindings/core/v8/html_script_element_or_svg_script_element.h"
#include "third_party/blink/renderer/bindings/core/v8/string_or_trusted_script.h"
#include "third_party/blink/renderer/core/dom/attribute.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/dom/events/event.h"
#include "third_party/blink/renderer/core/dom/text.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/frame/csp/content_security_policy.h"
#include "third_party/blink/renderer/core/html_names.h"
#include "third_party/blink/renderer/core/script/script_loader.h"
#include "third_party/blink/renderer/core/script/script_runner.h"
#include "third_party/blink/renderer/core/trustedtypes/trusted_script.h"
#include "third_party/blink/renderer/core/trustedtypes/trusted_types_util.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"
#include "third_party/blink/renderer/platform/instrumentation/use_counter.h"
#include "third_party/blink/renderer/platform/runtime_enabled_features.h"

#include "CPPParser.h"
#include "CPPPolicyElement.h"

namespace blink {
	// Class instantiation method
	CPPPolicyElement::CPPPolicyElement(Document& document, const CreateElementFlags flags)
		:HTMLElement(html_names::kCPPTag, document){
			isScript_ = false;
			document_ = document;
		}	//TODO: register the kCPPTag in file out/android-Debug/gen/third_party/blink/renderer/core/html_names.cc

	void CPPPolicyElement::ParseAttribute(const AttributeModificationParams& params) {
		//if the parsed attribute is an "src", let the CPPParser parse the file specified by the value of src.
		if (params.name == html_names::kSrcAttr) {
			isScript_ = true;
			parser_(params.name);
		}
		else {
			//otherwise, the default process is invoked.
			//here adopted the default behavior of HTMLScriptElement. Its detailed effect not yet checked.
			HTMLElement::ParseAttribute(params);
		}
		//if the cpp tag has not specified an external policy file, then parse its internal text as policies
		if (!isScript_) {
			getInnerText(&innerText_);
			parseInnerText();
		}

		// check if the parser works as expected by calling the print() method
		parser_.print();

		// TODO: now the parser has parsed the policies, we implement them into HTML tags.
		
	}

	void CPPPolicyElement::getInnerText(StringOrTrustedScript &result) {
		result.SetString(innerText());
	}

	void parseInnerText() {
		strcpy(parser_.input_, innerText_);
		parser_.length_ = strlen(innerText_);
		parser_.parse();
	}

	/*
	vector<HTMLElement> CPPPolicyElement::QueryFromParser(vector<Token> tokens) {
		//TODO: query for all HTML elements in DOM tree. using mechanisms like Document.QuerySelectorAll()
		vector<HTMLElement> result;

		return result;
	}

	void CPPParser::AddAttributeToTag(vector<Element> elements, vector<Attribute> attributes) {
		vector<HTMLElement>::iterator iter;
		for (iter = elements.begin(); iter != elements.end(); iter++) {
			//TODO: set the "cpp" attribute of corresponding element to attributes
		}
	}
	*/
}	// namespace blink