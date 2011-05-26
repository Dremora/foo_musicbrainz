#include "Query.h"
#include "RequestThread.h"

class CTaggerDialog : public CDialogImpl<CTaggerDialog>
{
private:
	CListViewCtrl release_list;
	CListViewCtrl track_list;
	CComboBox type;
	CComboBox status;
	CEdit artist;
	CEdit album;
	CEdit date;
	CEdit disc;
	CEdit discsubtitle;
	CEdit url;
	ReleaseList *mbc;
	track_list_view_edit track_list_view;

public:
	enum { IDD = IDD_TAGGER };

	CTaggerDialog(foo_musicbrainz::Query *query, ReleaseList *_mbc) : CDialogImpl<CTaggerDialog>(), mbc(_mbc)
	{
		Create(core_api::get_main_window());
		threaded_process::g_run_modeless(new service_impl_t<foo_musicbrainz::RequestThread>(query, m_hWnd, mbc),
			threaded_process::flag_show_progress | threaded_process::flag_show_abort, m_hWnd, "Quering information from MusicBrainz");
	}

	BEGIN_MSG_MAP(CTaggerDialog)
		MSG_WM_INITDIALOG(OnInitDialog)
		MESSAGE_HANDLER(WM_FOO_MB_UPDATE_RELEASES_LIST, OnUpdateReleaseList)
		MESSAGE_HANDLER(WM_FOO_MB_UPDATE_RELEASE, OnUpdateRelease)
		MSG_WM_CLOSE(OnClose)
		NOTIFY_HANDLER_EX(IDC_RELEASE_LIST, LVN_ITEMCHANGED, OnReleaseListChange)
		NOTIFY_HANDLER_EX(IDC_TRACK_LIST, NM_CLICK, OnTrackListClick)
		NOTIFY_HANDLER_EX(IDC_URL, NM_CLICK, OnLink)
		NOTIFY_HANDLER_EX(IDC_URL, NM_RETURN, OnLink)
		COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER_EX(IDOK, OnOk)
		COMMAND_HANDLER_EX(IDC_TYPE, CBN_SELENDOK, OnTypeChange)
		COMMAND_HANDLER_EX(IDC_STATUS, CBN_SELENDOK, OnStatusChange)
		COMMAND_HANDLER_EX(IDC_ARTIST, EN_UPDATE, OnArtistUpdate)
		COMMAND_HANDLER_EX(IDC_ALBUM, EN_UPDATE, OnAlbumUpdate)
		COMMAND_HANDLER_EX(IDC_DATE, EN_UPDATE, OnDateUpdate)
		COMMAND_HANDLER_EX(IDC_DISC, EN_UPDATE, OnDiscUpdate)
		COMMAND_HANDLER_EX(IDC_DISCSUBTITLE, EN_UPDATE, OnDiscSubtitleUpdate)
		//CHAIN_MSG_MAP(CDialogImpl<CTaggerDialog>)
	END_MSG_MAP()

	bool OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
	{
		static_api_ptr_t<modeless_dialog_manager>()->add(m_hWnd);
		release_list = GetDlgItem(IDC_RELEASE_LIST);
		track_list = GetDlgItem(IDC_TRACK_LIST);
		type = GetDlgItem(IDC_TYPE);
		status = GetDlgItem(IDC_STATUS);
		url = GetDlgItem(IDC_URL);
		artist = GetDlgItem(IDC_ARTIST);
		album = GetDlgItem(IDC_ALBUM);
		date = GetDlgItem(IDC_DATE);
		disc = GetDlgItem(IDC_DISC);
		discsubtitle = GetDlgItem(IDC_DISCSUBTITLE);
		track_list_view.Attach(track_list, mbc);
		
		// List view styles
		release_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);
		track_list.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

		// Adding release list columns
		listview_helper::insert_column(release_list, 0, "Artist", 115);
		listview_helper::insert_column(release_list, 1, "Release", 115);
		listview_helper::insert_column(release_list, 2, "Date", 49);

		// Adding track list columns
		// Fake column
		listview_helper::insert_column(track_list, 0, "", 0);
		track_list.InsertColumn(1, L"#", LVCFMT_RIGHT, 30);
		track_list.DeleteColumn(0);
		listview_helper::insert_column(track_list, 2, "Title", 260);

		for (int i = 0; i < MB_RELEASE_TYPES; i++)
		{
			pfc::stringcvt::string_os_from_utf8 str(Release::Types[i]);
			type.AddString(str);
		}
		for (int i = 0; i < MB_RELEASE_STATUSES; i++)
		{
			pfc::stringcvt::string_os_from_utf8 str(Release::Statuses[i]);
			status.AddString(str);
		}
		return true;
	}

	LRESULT OnUpdateReleaseList(UINT Message, WPARAM wParam, LPARAM lParam, BOOL bHandled)
	{
		for (unsigned int i = 0; i < mbc->getReleasesCount(); i++)
		{
			listview_helper::insert_item(release_list, i, mbc->getRelease(i)->getArtist(), NULL);
			listview_helper::set_item_text(release_list, i, 1, mbc->getRelease(i)->getTitle());
			listview_helper::set_item_text(release_list, i, 2, mbc->getRelease(i)->getDate());
		}

		Release *release = mbc->getRelease();
		char track_number_str[10];

		for (unsigned int i = 0; i < release->getTracksCount(); i++)
		{
			sprintf(track_number_str, "%u", i+1);
			listview_helper::insert_item(track_list, i, track_number_str, NULL);
		}
		SendMessage(WM_FOO_MB_UPDATE_RELEASE, 0, 0);
		return 0;
	}

	LRESULT OnUpdateRelease(UINT Message, WPARAM wParam, LPARAM lParam, BOOL bHandled)
	{
		Release *release = mbc->getRelease();

		uSetWindowText(artist, release->getArtist());
		uSetWindowText(album, release->getTitle());
		uSetWindowText(date, release->getDate());
		uSetWindowText(disc, release->getDisc());
		uSetWindowText(discsubtitle, release->getDiscSubtitle());

		type.SetCurSel(mbc->getRelease()->getType());
		status.SetCurSel(mbc->getRelease()->getStatus());

		// VA?
		if (track_list.GetColumnWidth(2) && !release->va)
		{
			track_list.SetColumnWidth(1, 260);
			track_list.DeleteColumn(2);
		}
		else if (!track_list.GetColumnWidth(2) && release->va)
		{
			track_list.SetColumnWidth(1, 195);
			listview_helper::insert_column(track_list, 2, "Track artist", 130);
		}

		// Tracks
		for (int iItem = 0; iItem < (int)release->getTracksCount(); iItem++)
		{
			listview_helper::set_item_text(track_list, iItem, 1, release->getTrack(iItem)->getTitle());
			listview_helper::set_item_text(track_list, iItem, 2, release->getTrack(iItem)->getArtist());
		}

		// Link
		pfc::string8 url_string = "<a href=\"http://musicbrainz.org/release/";
		url_string += release->getId();
		url_string += ".html\">MusicBrainz release page</a>";
		uSetWindowText(url, url_string);
		return 0;
	}

	LRESULT OnTrackListClick(LPNMHDR pnmh)
	{
		if (((LPNMITEMACTIVATE)pnmh)->iItem != -1 && ((LPNMITEMACTIVATE)pnmh)->iSubItem != 0)
		{
			track_list_view.Start(((LPNMITEMACTIVATE)pnmh)->iItem, ((LPNMITEMACTIVATE)pnmh)->iSubItem);
		}
		return 0;
	}

	LRESULT OnReleaseListChange(LPNMHDR pnmh)
	{
		if (((LPNMLISTVIEW)pnmh)->iItem != -1 && ((LPNMLISTVIEW)pnmh)->uChanged & LVIS_DROPHILITED && ((LPNMLISTVIEW)pnmh)->uNewState & LVIS_SELECTED)
		{
			if (mbc->getCurrentRelease() != ((LPNMITEMACTIVATE)pnmh)->iItem)
			{
				if (track_list_view.IsActive()) track_list_view.Abort();
				mbc->setCurrentRelease(((LPNMITEMACTIVATE)pnmh)->iItem);
				SendMessage(WM_FOO_MB_UPDATE_RELEASE, 0, 0);
			}
		}
		return 0;
	}

	LRESULT OnLink(LPNMHDR pnmh)
	{
		ShellExecute(NULL, L"open", ((PNMLINK)pnmh)->item.szUrl, NULL, NULL, SW_SHOW);
		return 0;
	}

	void OnClose()
	{
		DestroyWindow();
	}

	void OnFinalMessage(HWND hwnd)
	{
		static_api_ptr_t<modeless_dialog_manager>()->remove(m_hWnd);
		if (mbc) delete mbc;
		delete this;
	}

	void OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		DestroyWindow();
	}

	void OnOk(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		static_api_ptr_t<metadb_io_v2>()->update_info_async(*mbc->getData(),new service_impl_t<foo_mb_file_info_filter_impl>(mbc),core_api::get_main_window(), metadb_io_v2::op_flag_delay_ui, NULL);
		mbc = NULL;
		DestroyWindow();
	}

	void OnTypeChange(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		mbc->getRelease()->setType(type.GetCurSel());
	}

	void OnStatusChange(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		mbc->getRelease()->setStatus(status.GetCurSel());
	}

	void OnArtistUpdate(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		pfc::string8 str;
		uGetWindowText(artist, str);
		mbc->getRelease()->setArtist(str);
		listview_helper::set_item_text(release_list, mbc->getCurrentRelease(), 0, str);
	}

	void OnAlbumUpdate(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		pfc::string8 str;
		uGetWindowText(album, str);
		mbc->getRelease()->setTitle(str);
		listview_helper::set_item_text(release_list, mbc->getCurrentRelease(), 1, str);
	}

	void OnDateUpdate(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		pfc::string8 str;
		uGetWindowText(date, str);
		mbc->getRelease()->setDate(str);
		listview_helper::set_item_text(release_list, mbc->getCurrentRelease(), 2, str);
	}

	void OnDiscUpdate(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		mbc->getRelease()->setDisc(uGetDlgItemText(m_hWnd, nID).get_ptr());
	}

	void OnDiscSubtitleUpdate(UINT uNotifyCode, int nID, CWindow wndCtl)
	{
		mbc->getRelease()->setDiscSubtitle(uGetDlgItemText(m_hWnd, nID).get_ptr());
	}
};
