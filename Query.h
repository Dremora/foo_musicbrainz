#pragma once

#include "foo_musicbrainz.h"
#include "Metadata.h"

namespace foo_musicbrainz {
	class Query {
	public:
		Query(pfc::string8 id, pfc::string8 &includes = pfc::string8());
		Metadata *perform();

	private:
		pfc::string8 url_encode(pfc::string8 &);
		void get(pfc::string8 &buffer);
		ticpp::Element *parse(pfc::string8 &buffer, ticpp::Document &xml);

		pfc::string8 url;
	};
}
