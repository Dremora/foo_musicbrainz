#pragma once

#include "foo_musicbrainz.h"

namespace foo_musicbrainz {
	template <class T>
	class EntityList {
	public:
		void add(T *item) {
			items.add_item(item);
		};

		size_t count() const{
			return items.get_count();
		}

		T *operator[](t_size index) const {
			if (index >= items.get_count()) {
				throw pfc::exception_overflow();
			}
			return items[index];
		}

		EntityList() {};
		~EntityList() {
			for (unsigned int i = 0; i < items.get_count(); i++) {
				delete items[i];
			}
		}

	protected:
		pfc::list_t<T *> items;
	};
}
