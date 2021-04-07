#ifndef THIRD_PARTY_BLINK_RENDERER_CORE_HTML_HTML_SCRIPT_ELEMENT_H_
#define THIRD_PARTY_BLINK_RENDERER_CORE_HTML_HTML_SCRIPT_ELEMENT_H_

#include "third_party/blink/renderer/core/core_export.h"
#include "third_party/blink/renderer/core/dom/create_element_flags.h"
#include "third_party/blink/renderer/core/html/html_element.h"
#include "third_party/blink/renderer/core/script/script_element_base.h"
#include "third_party/blink/renderer/core/script/script_loader.h"
#include "third_party/blink/renderer/platform/bindings/parkable_string.h"

#include "CPPParser.h"

namespace blink {
	class CORE_EXPORT CPPPolicyElement : public HTMLElement{
		public:
			CPPPolicyElement(Document&, const CreateElementFlags);
			void ParseAttribute(const AttributeModificationParams&) override;
			void QueryFromParser(vector<Token>);	//query for elements corresponding to the tag tokens of the parsed policy
			void AddAttributeToTag(vector<HTMLElement>, vector<Attribute>) //for the HTML tags passed as argument, add the policy directives specified by the policy file to the selected tag.
		private:
			Document *document_;
			CPPParser parser_;
			bool isScript_;
			StringOrTrustedScript innerText_;
	};	//class HTMLScriptElement;
}	// namespace blink