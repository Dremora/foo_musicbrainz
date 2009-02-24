#include "foo_musicbrainz.h"

BOOL CALLBACK TaggerDialogProc(HWND tagger_dialog, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
		{
			HWND release_list = GetDlgItem(tagger_dialog, IDC_RELEASE_LIST);
			HWND track_list = GetDlgItem(tagger_dialog, IDC_TRACK_LIST);
			
			LVCOLUMN column_item;
			
			// List view styles
			ListView_SetExtendedListViewStyleEx(release_list, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
			ListView_SetExtendedListViewStyleEx(track_list, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

			// Adding release list columns
			column_item.mask = LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 
			wchar_t *release_list_columns[] = { L"Artist", L"Release", L"Date" };
			int col_size[] = { 180, 180, 70 };
			for (int i = 0; i < 3; i++) 
			{ 
				column_item.iSubItem = i;
				column_item.pszText = release_list_columns[i];
				column_item.cx = col_size[i];
				ListView_InsertColumn(release_list, i, &column_item);
			}

			// Adding track list columns
			// Fake column
			column_item.mask = 0;
			ListView_InsertColumn(track_list, 0, &column_item);

			column_item.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
			column_item.fmt = LVCFMT_RIGHT;
			column_item.cx = 30;
			column_item.pszText = L"#";
			ListView_InsertColumn(track_list, 1, &column_item);

			column_item.mask = LVCF_WIDTH | LVCF_TEXT;
			column_item.cx = 390;
			column_item.pszText = L"Title";
			ListView_InsertColumn(track_list, 2, &column_item);

			ListView_DeleteColumn(track_list, 0);

			//column_item.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM; 

			//column_item.iSubItem = 0;
			//column_item.pszText = L"#";
			//column_item.cx = 100;
			//if (i == 0) column_item.fmt = LVCFMT_RIGHT;
			//else column_item.fmt = LVCFMT_LEFT;
			//ListView_InsertColumn(track_list, i, &column_item);

			//wchar_t *track_list_columns[] = { L"#", L"Title" };
			//for (int i = 0; i < 2; i++) 
			//{ 
			//	column_item.iSubItem = i;
			//	column_item.pszText = track_list_columns[i];
			//	column_item.cx = 100;
			//	if (i == 0) column_item.fmt = LVCFMT_RIGHT;
			//	else column_item.fmt = LVCFMT_LEFT;
			//	ListView_InsertColumn(track_list, i, &column_item);
			//}

			break;
		}

	case WM_FOO_MB_UPDATE_RELEASES_LIST:
		{
			HWND release_list = GetDlgItem(tagger_dialog, IDC_RELEASE_LIST);
			LVITEM list_item;
			uconvert str;
			mbCollection *mbc = (mbCollection *)GetProp(tagger_dialog, L"Collection");

			list_item.mask = LVIF_TEXT;
			list_item.iSubItem = 0;

			ListView_DeleteAllItems(release_list);
			for (list_item.iItem = 0; list_item.iItem < (int)mbc->getReleasesCount(); list_item.iItem++)
			{
				list_item.pszText = str.ToUtf16(mbc->getRelease(list_item.iItem)->getArtist());
				ListView_InsertItem(release_list, &list_item);
				ListView_SetItemText(release_list, list_item.iItem, 1, str.ToUtf16(mbc->getRelease(list_item.iItem)->getTitle()));
				ListView_SetItemText(release_list, list_item.iItem, 2, str.ToUtf16(mbc->getRelease(list_item.iItem)->getDate()));
			}

			mbRelease *release = mbc->getRelease(mbc->getCurrentRelease());
			wchar_t track_number_str[10];
			HWND track_list = GetDlgItem(tagger_dialog, IDC_TRACK_LIST);

			list_item.mask = LVIF_TEXT;
			list_item.iSubItem = 0;

			for (list_item.iItem = 0; list_item.iItem < (int)release->getTracksCount(); list_item.iItem++)
			{
				//column 1 - track_number
				wsprintf(track_number_str, L"%u", list_item.iItem+1);
				list_item.pszText = track_number_str;
				ListView_InsertItem(track_list, &list_item);

				//column 2 - title
				ListView_SetItemText(track_list, list_item.iItem, 1, str.ToUtf16(release->getTrack(list_item.iItem)->getTitle()));
			}

			if (release->getTracksCount() == 0)
			{
				pfc::string8 url = "ws/1/release/";
				url += URLEncode(mbc->getRelease(mbc->getCurrentRelease())->getId());
				url += "?type=xml&inc=tracks"; 
				threaded_process::g_run_modeless(new service_impl_t<foo_mb_request_thread>(url.get_ptr(), tagger_dialog, foo_mb_request_thread::flag_tracks_only, mbc->getCurrentRelease()), threaded_process::flag_show_progress | threaded_process::flag_show_abort, core_api::get_main_window(), "Quering information from MusicBrainz");
			}

			pfc::string8 url;
			url = "<a href=\"http://musicbrainz.org/release/";
			url += release->getId();
			url += ".html\">MusicBrainz release page</a>";
			uSetDlgItemText(tagger_dialog, IDC_URL, url);
			break;
		}

	case WM_FOO_MB_UPDATE_RELEASE:
		{
			mbCollection *mbc = (mbCollection *)GetProp(tagger_dialog, L"Collection");
			mbRelease *release = mbc->getRelease(mbc->getCurrentRelease());
			uconvert str;
			HWND track_list = GetDlgItem(tagger_dialog, IDC_TRACK_LIST);

			for (int iItem = 0; iItem < (int)release->getTracksCount(); iItem++)
			{
				//column 2 - title
				ListView_SetItemText(track_list, iItem, 1, str.ToUtf16(release->getTrack(iItem)->getTitle()));
			}

			if (release->getTracksCount() == 0)
			{
				pfc::string8 url = "ws/1/release/";
				url += URLEncode(mbc->getRelease(mbc->getCurrentRelease())->getId());
				url += "?type=xml&inc=tracks"; 
				threaded_process::g_run_modeless(new service_impl_t<foo_mb_request_thread>(url.get_ptr(), tagger_dialog, foo_mb_request_thread::flag_tracks_only, mbc->getCurrentRelease()), threaded_process::flag_show_progress | threaded_process::flag_show_abort, core_api::get_main_window(), "Quering information from MusicBrainz");
			}

			pfc::string8 url;
			url = "<a href=\"http://musicbrainz.org/release/";
			url += release->getId();
			url += ".html\">MusicBrainz release page</a>";
			uSetDlgItemText(tagger_dialog, IDC_URL, url);
			break;
		}

	case WM_CLOSE:
		DestroyWindow(tagger_dialog);
		break;

	case WM_DESTROY:
		delete GetProp(tagger_dialog, L"Collection");
		RemoveProp(tagger_dialog, L"Collection");
		break;
		
	case WM_NOTIFY:// 
	/*	if (((LPNMHDR)lParam)->code <= LVN_FIRST && ((LPNMHDR)lParam)->code >= LVN_LAST && ((LPNMHDR)lParam)->code != LVN_ITEMCHANGING)
		{
			console::printf("0x%08x", ((LPNMHDR)lParam)->code);
		}*/
		switch (((LPNMHDR)lParam)->code)
		{
		case NM_DBLCLK:
			if (((LPNMITEMACTIVATE)lParam)->iItem != -1 && ((LPNMHDR)lParam)->hwndFrom == GetDlgItem(tagger_dialog, IDC_RELEASE_LIST))
			{
				((mbCollection *)GetProp(tagger_dialog, L"Collection"))->getReleasesTable()->start(((LPNMITEMACTIVATE)lParam)->iItem, ((LPNMITEMACTIVATE)lParam)->iSubItem);
			}
			break;

		case LVN_ITEMCHANGED:
			if (((LPNMLISTVIEW)lParam)->hdr.hwndFrom == GetDlgItem(tagger_dialog, IDC_RELEASE_LIST) && ((LPNMLISTVIEW)lParam)->iItem != -1 && ((LPNMLISTVIEW)lParam)->uChanged & LVIS_DROPHILITED && ((LPNMLISTVIEW)lParam)->uNewState & LVIS_SELECTED)
			{
				if (((mbCollection *)GetProp(tagger_dialog, L"Collection"))->getCurrentRelease() != ((LPNMITEMACTIVATE)lParam)->iItem)
				{
					((mbCollection *)GetProp(tagger_dialog, L"Collection"))->setCurrentRelease(((LPNMITEMACTIVATE)lParam)->iItem);
					SendMessage(tagger_dialog, WM_FOO_MB_UPDATE_RELEASE, 0, 0);
				}
			}
			break;

		case LVN_ITEMCHANGING:
			console::printf("Selected item %u.%u. Changed: %u", ((LPNMITEMACTIVATE)lParam)->iItem, ((LPNMITEMACTIVATE)lParam)->iSubItem, ((LPNMITEMACTIVATE)lParam)->uChanged);
			break;

		case NM_CLICK:
			if (((LPNMITEMACTIVATE)lParam)->iItem != -1 && ((LPNMHDR)lParam)->hwndFrom == GetDlgItem(tagger_dialog, IDC_TRACK_LIST) && ((LPNMITEMACTIVATE)lParam)->iSubItem != 0)
			{
				((mbCollection *)GetProp(tagger_dialog, L"Collection"))->getTracksTable()->start(((LPNMITEMACTIVATE)lParam)->iItem, ((LPNMITEMACTIVATE)lParam)->iSubItem);
				break;
			}

		case NM_RETURN:
			{
				if ((((LPNMHDR)lParam)->hwndFrom == GetDlgItem(tagger_dialog, IDC_URL)) && (((PNMLINK)lParam)->item.iLink == 0))
				{	
					ShellExecute(NULL, L"open", ((PNMLINK)lParam)->item.szUrl, NULL, NULL, SW_SHOW);
				}
				break;
			}

		default:
			return FALSE;
		}

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			DestroyWindow(tagger_dialog);
			break;

		case IDOK:
			{
				mbRelease *release = ((mbCollection *)GetProp(tagger_dialog, L"Collection"))->getRelease(((mbCollection *)GetProp(tagger_dialog, L"Collection"))->getCurrentRelease());

				pfc::list_t<const file_info*> info_list;
				char track_number_str[10];
				file_info_impl *info = new file_info_impl[release->getTracksCount()];

				metadb_handle_list *data = ((mbCollection *)GetProp(tagger_dialog, L"Collection"))->getData();
				
				for (unsigned int i = 0; i < release->getTracksCount(); i++)
				{
					data->get_item(i)->get_info(info[i]);
					info[i].meta_set("ARTIST", release->getArtist());
					info[i].meta_set("ALBUM", release->getTitle());
					info[i].meta_set("DATE", release->getDate());
					info[i].meta_set("TITLE", release->getTrack(i)->getTitle());
					sprintf(track_number_str, "%u", i+1);
					info[i].meta_set("TRACKNUMBER", track_number_str);
					sprintf(track_number_str, "%u", release->getTracksCount());
					info[i].meta_set("TOTALTRACKS", track_number_str);
					info_list.add_item(&info[i]);
				}
				static_api_ptr_t<metadb_io_v2>()->update_info_async_simple(*data, info_list, core_api::get_main_window(), metadb_io_v2::op_flag_delay_ui, NULL);
				DestroyWindow(tagger_dialog);
				break;
			}

		default:
			return FALSE;
		}

	default:
		return FALSE;
	}
	return TRUE;
}
