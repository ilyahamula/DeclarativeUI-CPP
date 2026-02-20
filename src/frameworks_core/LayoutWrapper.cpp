#include "frameworks_core/LayoutWrapper.hpp"
#include "frameworks_core/LayoutFlags.hpp"
#include "frameworks_core/ControlWrapper.hpp"

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#ifdef USE_LOGGER
std::string LayoutWrapper::indent()
{
	return std::string(s_depth, '\t');
}
#endif

LayoutWrapper::~LayoutWrapper() = default;

#ifdef USE_WX

LayoutWrapper::LayoutWrapper(Orientation orient)
	: m_orientation(orient)
{
#ifdef USE_LOGGER
	Logger::instance().log(indent() + (orient == Orientation::Horizontal
		? "LayoutWrapper::LayoutWrapper() Horizontal\t-> new wxBoxSizer()\n"
		: "LayoutWrapper::LayoutWrapper() Vertical\t-> new wxBoxSizer()\n"));
	++s_depth;
#endif
	m_nativeSizer = new wxBoxSizer(orient == Orientation::Horizontal ? wxHORIZONTAL : wxVERTICAL);
}

LayoutWrapper::LayoutWrapper(wxSizer* sizer)
	: m_nativeSizer(sizer)
{
}

void LayoutWrapper::add(LayoutFlags& flags)
{
}

void LayoutWrapper::add(LayoutWrapper* stack, LayoutFlags& flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(indent() + "LayoutWrapper::add(stack)\t-> wxSizer->Add()\n");
#endif
	m_nativeSizer->Add(stack->nativeHandle(), flags.wx());
}

void LayoutWrapper::add(ControlWrapper* widget, LayoutFlags& flags)
{
	m_nativeSizer->Add(widget->nativeHandle(), flags.wx());
}

void LayoutWrapper::finilizeLayout()
{
#ifdef USE_LOGGER
	--s_depth;
#endif
}

wxSizer* LayoutWrapper::nativeHandle() const
{
	return m_nativeSizer;
}
#elif defined(USE_QT)
#elif defined(USE_IMGUI)
#include "imgui.h"
#include <algorithm>
#include <unordered_map>

namespace
{

// Frame-delayed height cache for CenterVertical layout flag.
// Each horizontal layout gets a sequential ID per frame. Heights measured
// on frame N are used to compute vertical centering offsets on frame N+1.
class CenterVCache
{
	CenterVCache() = default;
public:
	static CenterVCache& instance()
	{
		static CenterVCache s;
		return s;
	}

	int nextLayoutId()
	{
		int frame = ImGui::GetFrameCount();
		if (frame != m_lastFrame)
		{
			m_nextId = 0;
			m_lastFrame = frame;
		}
		return m_nextId++;
	}

	float centeringOffset(int layoutId, int widgetIndex) const
	{
		auto it = m_cache.find(layoutId);
		if (it != m_cache.end() && widgetIndex < (int)it->second.itemHeights.size())
		{
			float maxH = it->second.maxHeight;
			float itemH = it->second.itemHeights[widgetIndex];
			return (maxH - itemH) / 2.0f;
		}
		return 0.0f;
	}

	void save(int layoutId, std::vector<float> heights)
	{
		if (heights.empty())
			return;
		Entry& e = m_cache[layoutId];
		e.maxHeight = *std::max_element(heights.begin(), heights.end());
		e.itemHeights = std::move(heights);
	}

private:
	struct Entry {
		float maxHeight = 0;
		std::vector<float> itemHeights;
	};
	std::unordered_map<int, Entry> m_cache;
	int m_nextId = 0;
	int m_lastFrame = -1;
};

} // unnamed namespace

LayoutWrapper::LayoutWrapper(Orientation orient)
	: m_orientation(orient)
{
#ifdef USE_LOGGER
	Logger::instance().log(indent() + (m_orientation == Orientation::Horizontal
		? "LayoutWrapper::LayoutWrapper() Horizontal\t-> ImGui::BeginGroup()\n"
		: "LayoutWrapper::LayoutWrapper() Vertical\t-> ImGui::BeginGroup()\n"));
	++s_depth;
#endif
	m_centerVLayoutId = CenterVCache::instance().nextLayoutId();

	ImGui::BeginGroup();
}

void LayoutWrapper::add(LayoutFlags& flags)
{
	bool wasFirstChild = m_firstChild;

	// Measure previous item's height for vertical centering cache
	if (!m_firstChild && m_orientation == Orientation::Horizontal)
	{
		m_currentHeights.push_back(ImGui::GetItemRectSize().y);
	}

	if (!m_firstChild && m_orientation == Orientation::Horizontal)
	{
		float gap = std::max((float)m_lastBorderRight, (float)flags.borderLeft());
		if (gap > 0)
			ImGui::SameLine(0, gap);
		else
			ImGui::SameLine();
	}
	m_firstChild = false;

	if (flags.borderTop() > 0 && m_orientation == Orientation::Vertical)
		ImGui::Dummy(ImVec2(0, (float)flags.borderTop()));

	if (flags.borderLeft() > 0 && m_orientation == Orientation::Vertical)
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (float)flags.borderLeft());

	// Apply vertical centering offset from previous frame's cache
	if (m_orientation == Orientation::Horizontal && flags.centerVertical())
	{
		float offset = CenterVCache::instance().centeringOffset(m_centerVLayoutId, m_widgetIndex);
		if (offset > 1.0f)
		{
			// For the first item on a line, anchor the row baseline with a
			// zero-size dummy so that SetCursorPosY doesn't shift
			// CursorPosPrevLine.y (which SameLine uses for subsequent items).
			if (wasFirstChild)
			{
				ImGui::Dummy(ImVec2(0, 0));
				ImGui::SameLine(0, 0);
			}
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offset);
		}
	}

	m_widgetIndex++;
	m_lastBorderRight = flags.borderRight();
}

void LayoutWrapper::add(LayoutWrapper* stack, LayoutFlags& flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(indent() + (m_orientation == Orientation::Horizontal
		? "LayoutWrapper::add(stack) Horizontal\n"
		: "LayoutWrapper::add(stack) Vertical\n"));
#endif
}

void LayoutWrapper::add(ControlWrapper* widget, LayoutFlags& flags)
{
#ifdef USE_LOGGER
	Logger::instance().log(indent() + (m_orientation == Orientation::Horizontal
		? "LayoutWrapper::add(widget) Horizontal\n"
		: "LayoutWrapper::add(widget) Vertical\n"));
#endif
	bool wasFirstChild = m_firstChild;
	add(flags);

	if (m_orientation == Orientation::Vertical)
	{
		if (flags.expand())
			ImGui::SetNextItemWidth(-FLT_MIN);
	}
	else
	{
		if (flags.proportion() > 0 && !wasFirstChild)
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	}
}

void LayoutWrapper::saveCenterVCache()
{
	if (m_orientation == Orientation::Horizontal && !m_currentHeights.empty())
	{
		// Measure the last item
		m_currentHeights.push_back(ImGui::GetItemRectSize().y);
		CenterVCache::instance().save(m_centerVLayoutId, std::move(m_currentHeights));
		// m_currentHeights is now empty, so a second call is a no-op
	}
}

void LayoutWrapper::finilizeLayout()
{
	saveCenterVCache();
#ifdef USE_LOGGER
	--s_depth;
	Logger::instance().log(indent() + (m_orientation == Orientation::Horizontal
		? "LayoutWrapper::finilizeLayout() Horizontal\t-> ImGui::EndGroup()\n"
		: "LayoutWrapper::finilizeLayout() Vertical\t-> ImGui::EndGroup()\n"));
#endif
	ImGui::EndGroup();
}
#endif
