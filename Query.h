#pragma once

#include "foo_musicbrainz.h"
#include "Release.h"

namespace foo_musicbrainz {
	class Query {
	public:
		Query(pfc::string8 id);
		Release *get_release();
		// void perform();

	private:
		pfc::string8 url_encode(pfc::string8 &);
		void get(pfc::string8 &buffer);
		void parse(pfc::string8 &buffer, ticpp::Document &xml);

		pfc::string8 url;
	};
}
