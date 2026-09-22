#include "frameworks_core/imgui/FileBrowserPopup.hpp"

#include "imgui.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

namespace
{

// One name in the listing. Directories sort first and then both groups sort by
// name, which is what every native browser does and what makes the list
// navigable without a header row.
struct Entry
{
	std::string name;
	bool isDirectory = false;
};

// The live browser. A file-static singleton rather than per-widget state keyed
// like a snapshot: a modal browser is singular by construction, so there is
// nothing to key it by.
struct State
{
	bool active = false;       // a request is up
	bool opened = false;       // OpenPopup has been called for it

	// WHICH window scope owns the popup, and when it last drew.
	//
	// drawPending() runs once per top-level window, and a popup's real ImGui id
	// is hashed against the CURRENT window's id stack -- so the same kPopupId
	// resolves differently in each of them. Without recording the scope that
	// opened the popup, the second window of a frame would ask about an id that
	// is not open and mistake it for a dismissal. lastDrawnFrame is what lets
	// the popup be re-claimed if the window that owned it goes away.
	ImGuiID popupId = 0;
	int lastDrawnFrame = -1;

	std::string title;
	FileMode mode = FileMode::Open;
	std::vector<FileFilter> filters;
	std::function<void(const std::string&)> onResult;

	fs::path directory;        // what the listing shows
	std::string fileName;      // the Save name field / the picked file
	int selected = -1;         // index into `entries`
	int filterIndex = 0;
	bool listingStale = true;
	std::vector<Entry> entries;
};

State& state()
{
	static State s;
	return s;
}

// Fixed id: the popup is opened and begun in the same window scope, and there
// is only ever one of them.
const char* const kPopupId = "##dui_file_browser";

// Every filesystem call here takes the error_code overload. A directory the
// process cannot read is ordinary -- /root, a stale mount, a permission-denied
// folder the user clicked into -- and must not throw out of a frame.
void refreshListing(State& s)
{
	s.entries.clear();
	s.selected = -1;
	s.listingStale = false;

	std::error_code ec;
	fs::directory_iterator it(s.directory, fs::directory_options::skip_permission_denied, ec);
	if (ec)
		return; // an unreadable directory lists as empty; ".." still gets the user out

	const FileFilter* filter = (!s.filters.empty() && s.filterIndex >= 0
		&& s.filterIndex < (int)s.filters.size())
		? &s.filters[(std::size_t)s.filterIndex]
		: nullptr;

	for (const fs::directory_entry& entry : it)
	{
		std::error_code dirEc;
		const bool isDirectory = entry.is_directory(dirEc);
		const std::string name = entry.path().filename().string();
		if (name.empty() || name.front() == '.')
			continue; // dotfiles stay hidden, as they do in the native dialogs

		// Directories are never filtered: the filter picks what you may CHOOSE,
		// not where you may go. Filtering folders would strand the user.
		if (!isDirectory)
		{
			if (s.mode == FileMode::Directory)
				continue;
			if (filter != nullptr && !filter->matches(name))
				continue;
		}
		s.entries.push_back(Entry { name, isDirectory });
	}

	std::sort(s.entries.begin(), s.entries.end(), [](const Entry& a, const Entry& b) {
		if (a.isDirectory != b.isDirectory)
			return a.isDirectory;
		return a.name < b.name;
	});
}

void navigateTo(State& s, const fs::path& directory)
{
	s.directory = directory;
	s.fileName.clear();
	s.listingStale = true;
}

// What the caller gets back. Save returns the typed name joined to the current
// directory even when no such file exists yet -- that is the whole point of
// Save -- while Open and Directory can only return something that is there.
std::string chosenPath(const State& s)
{
	if (s.mode == FileMode::Directory)
		return s.directory.string();
	if (s.fileName.empty())
		return std::string();
	return (s.directory / s.fileName).string();
}

bool canAccept(const State& s)
{
	return s.mode == FileMode::Directory || !s.fileName.empty();
}

// Retire the request and report. The callback is moved out and the state
// cleared BEFORE it runs: a handler is entitled to open the next browser, and
// it must not be cleared out from under itself when it does.
//
// `insidePopup` says whether there is a current popup to close. There is not
// when ImGui has already closed it for us -- which is the Escape path below.
void finish(State& s, const std::string& result, bool insidePopup = true)
{
	auto callback = std::move(s.onResult);
	s = State{};
	if (insidePopup)
		ImGui::CloseCurrentPopup();
	if (callback)
		callback(result);
}

// The breadcrumb: each ancestor of the current directory as its own button.
void drawBreadcrumb(State& s)
{
	std::vector<fs::path> chain;
	for (fs::path p = s.directory; !p.empty(); p = p.parent_path())
	{
		chain.push_back(p);
		if (p == p.parent_path())
			break; // the root is its own parent; stop rather than loop
	}
	std::reverse(chain.begin(), chain.end());

	for (std::size_t i = 0; i < chain.size(); ++i)
	{
		if (i > 0)
		{
			ImGui::SameLine(0, 2);
			ImGui::TextUnformatted("/");
			ImGui::SameLine(0, 2);
		}
		const std::string name = chain[i].filename().string();
		const std::string label = name.empty() ? chain[i].string() : name;
		ImGui::PushID((int)i);
		if (ImGui::SmallButton(label.c_str()))
			navigateTo(s, chain[i]);
		ImGui::PopID();
	}
}

} // unnamed namespace

void FileBrowser::request(const std::string& title, FileMode mode,
	std::vector<FileFilter> filters, std::string initialPath,
	std::function<void(const std::string&)> onResult)
{
	State& s = state();
	if (s.active)
		return; // one modal at a time; a second ask this frame is dropped

	s = State{};
	s.active = true;
	s.title = title.empty() ? std::string("Select a file") : title;
	s.mode = mode;
	s.filters = std::move(filters);
	s.onResult = std::move(onResult);

	// The initial path seeds both halves: its directory is where we open and,
	// for Save, its leaf name pre-fills the name field. A path that IS a
	// directory seeds the directory only.
	std::error_code ec;
	const fs::path initial = initialPath.empty() ? fs::path() : fs::path(initialPath);
	if (!initial.empty() && fs::is_directory(initial, ec) && !ec)
		s.directory = initial;
	else if (!initial.empty() && initial.has_parent_path()
		&& fs::is_directory(initial.parent_path(), ec) && !ec)
	{
		s.directory = initial.parent_path();
		if (mode != FileMode::Directory)
			s.fileName = initial.filename().string();
	}
	else
	{
		s.directory = fs::current_path(ec);
		if (ec)
			s.directory = fs::path("/");
	}
	s.listingStale = true;
}

bool FileBrowser::pending()
{
	return state().active;
}

void FileBrowser::drawPending()
{
	State& s = state();
	if (!s.active)
		return;

	// Which scope are we in? See State::popupId -- this function runs once per
	// top-level window and only ONE of them owns the popup.
	const ImGuiID id = ImGui::GetID(kPopupId);
	const int frame = ImGui::GetFrameCount();
	const bool stale = frame - s.lastDrawnFrame > 1;

	if (!s.opened || (id != s.popupId && stale))
	{
		// First draw, or the window that owned the popup has gone away and
		// this one is taking it over. Re-claiming rather than hanging matters:
		// a request nothing draws would block every later one forever, since
		// request() drops an ask while one is already active.
		ImGui::OpenPopup(kPopupId);
		s.opened = true;
		s.popupId = id;
		s.lastDrawnFrame = frame;
	}
	else if (id != s.popupId)
	{
		return; // another window's turn this frame, not ours
	}

	if (s.listingStale)
		refreshListing(s);

	ImGui::SetNextWindowSize(ImVec2(560, 420), ImGuiCond_Appearing);
	if (!ImGui::BeginPopupModal(kPopupId, nullptr, ImGuiWindowFlags_NoSavedSettings))
	{
		// ImGui closes a popup on Escape itself, without any button here
		// running -- so a popup that has gone away in OUR OWN scope is a
		// cancel. Reporting it is what keeps the request from staying active
		// forever and swallowing every later Browse click.
		finish(s, std::string(), false);
		return;
	}
	s.lastDrawnFrame = frame;

	ImGui::TextUnformatted(s.title.c_str());
	ImGui::Separator();
	drawBreadcrumb(s);

	const float footerHeight = ImGui::GetFrameHeightWithSpacing() * 2.0f
		+ ImGui::GetStyle().ItemSpacing.y;
	if (ImGui::BeginChild("##listing", ImVec2(0, -footerHeight), ImGuiChildFlags_Borders))
	{
		// ".." first and always, including in a directory the iterator could not
		// read -- it is the only way back out of one.
		if (s.directory.has_parent_path() && s.directory != s.directory.parent_path())
		{
			if (ImGui::Selectable("../", false, ImGuiSelectableFlags_AllowDoubleClick)
				&& ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				navigateTo(s, s.directory.parent_path());
			}
		}

		for (int i = 0; i < (int)s.entries.size(); ++i)
		{
			const Entry& entry = s.entries[(std::size_t)i];
			const std::string label = entry.isDirectory ? entry.name + "/" : entry.name;
			if (ImGui::Selectable(label.c_str(), s.selected == i,
				ImGuiSelectableFlags_AllowDoubleClick))
			{
				s.selected = i;
				if (!entry.isDirectory)
					s.fileName = entry.name;
				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (entry.isDirectory)
					{
						navigateTo(s, s.directory / entry.name);
						break; // the listing under us has just been invalidated
					}
					if (s.mode != FileMode::Directory)
					{
						finish(s, chosenPath(s));
						ImGui::EndChild();
						ImGui::EndPopup();
						return;
					}
				}
			}
		}
	}
	ImGui::EndChild();

	// The name field. Editable in Save (you are naming a file that may not
	// exist) and read-only in Open (you are picking one that does), which is
	// the same split the native dialogs make.
	if (s.mode != FileMode::Directory)
	{
		char buffer[512] = {};
		std::snprintf(buffer, sizeof(buffer), "%s", s.fileName.c_str());
		ImGui::SetNextItemWidth(-160.0f);
		const ImGuiInputTextFlags flags = s.mode == FileMode::Save
			? ImGuiInputTextFlags_None
			: ImGuiInputTextFlags_ReadOnly;
		if (ImGui::InputText("##filename", buffer, sizeof(buffer), flags))
			s.fileName = buffer;

		if (!s.filters.empty())
		{
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-1.0f);
			const std::string current = s.filters[(std::size_t)s.filterIndex].description;
			if (ImGui::BeginCombo("##filter", current.c_str()))
			{
				for (int i = 0; i < (int)s.filters.size(); ++i)
				{
					if (ImGui::Selectable(s.filters[(std::size_t)i].description.c_str(),
						s.filterIndex == i))
					{
						s.filterIndex = i;
						s.listingStale = true; // a new filter is a new listing
					}
				}
				ImGui::EndCombo();
			}
		}
	}
	else
	{
		ImGui::TextUnformatted(s.directory.string().c_str());
	}

	const char* acceptLabel = s.mode == FileMode::Save ? "Save" : "Open";
	ImGui::BeginDisabled(!canAccept(s));
	const bool accepted = ImGui::Button(acceptLabel, ImVec2(110, 0));
	ImGui::EndDisabled();
	ImGui::SameLine();
	// Escape is handled HERE rather than left to ImGui. ImGui only closes a
	// popup on Escape when keyboard nav is enabled (ImGuiConfigFlags_
	// NavEnableKeyboard), which is off by default and off in this project's
	// mains -- so relying on it would leave the browser with no keyboard
	// escape at all. IsKeyPressed reads the key state directly and needs no
	// such flag. The dismissal branch above stays as the safety net for a host
	// that DOES enable nav: it clears the request rather than double-reporting,
	// because finish() clears the state before anything else can see it.
	const bool cancelled = ImGui::Button("Cancel", ImVec2(110, 0))
		|| ImGui::IsKeyPressed(ImGuiKey_Escape);

	if (accepted)
		finish(s, chosenPath(s));
	else if (cancelled)
		finish(s, std::string()); // "" is the framework's cancel, as on wx and Qt

	ImGui::EndPopup();
}
