#pragma once

#include <wx/wx.h>
#include <optional>

template <typename W>
struct Widget
{
	Widget(wxWindowID id, std::string str)
		: m_id(id)
		, m_str(str)
	{
	}

	virtual ~Widget() = default;

	void createAndAdd(wxWindow* parent, wxSizer* sizer, wxSizerFlags flags)
	{
		sizer->Add(createWidget(parent, m_id, m_str, m_position, m_size, m_style), flags);
	}

	W& withFlags(wxSizerFlags flags)
	{
		m_flags = flags;
		return static_cast<W&>(*this);
	}

	W& withPosition(const wxPoint& pos)
	{
		m_position = pos;
		return static_cast<W&>(*this);
	}

	W& withSize(const wxSize& size)
	{
		m_size = size;
		return static_cast<W&>(*this);
	}

	W& withStyle(long style)
	{
		m_style = style;
		return static_cast<W&>(*this);
	}

private:
	virtual wxWindow* createWidget(
		wxWindow* parent, 
		wxWindowID id,
		const std::string& str,
		const wxPoint& pos,
		const wxSize& size,
		long style) = 0;

private:
	wxWindowID m_id;
	std::string m_str;
	std::optional<wxSizerFlags> m_flags;
	wxPoint m_position { wxDefaultPosition };
	wxSize m_size { wxDefaultSize };
	long m_style { 0 };
};

// Text  -----------------------------------------------------------
struct StaticText : Widget<StaticText>
{
	using super = Widget<StaticText>;

	explicit StaticText(wxWindowID id = wxID_ANY, std::string str = "")
		: super(id, str)
	{
	}

	explicit StaticText(const std::string& str)
		: super(wxID_ANY, str)
	{
	}
private:
	wxWindow* createWidget(
		wxWindow* parent,
		wxWindowID id,
		const std::string& str,
		const wxPoint& pos,
		const wxSize& size,
		long style) override
	{
		return new wxStaticText(parent, id, str, pos, size, style);
	}
};


// Text Control -----------------------------------------------------------
struct TextCtrl : Widget<TextCtrl>
{
	using super = Widget<TextCtrl>;

	explicit TextCtrl(wxWindowID id = wxID_ANY, std::string str = "")
		: super(id, str)
	{
	}

	explicit TextCtrl(const std::string& str)
		: super(wxID_ANY, str)
	{
	}
private:
	wxWindow* createWidget(
		wxWindow* parent,
		wxWindowID id,
		const std::string& str,
		const wxPoint& pos,
		const wxSize& size,
		long style) override
	{
		return new wxTextCtrl(parent, id, str, pos, size, style);
	}
};

// Button -----------------------------------------------------------
struct Button : Widget<Button>
{
	using super = Widget<Button>;

	explicit Button(wxWindowID id = wxID_ANY, std::string str = "")
		: super(id, str)
	{
	}

	explicit Button(const std::string& str)
		: super(wxID_ANY, str)
	{
	}
private:
	wxWindow* createWidget(
		wxWindow* parent,
		wxWindowID id,
		const std::string& str,
		const wxPoint& pos,
		const wxSize& size,
		long style) override
	{
		return new wxButton(parent, id, str, pos, size, style);
	}
};

// Slider -----------------------------------------------------------
struct Slider : Widget<Slider>
{
	using super = Widget<Slider>;
	
	explicit Slider(wxWindowID id, std::pair<int, int> range, std::optional<int> value = std::nullopt)
		: super(id, "")
		, m_value(value.value_or(range.first))
		, m_range(range)
	{
	}

	explicit Slider(std::pair<int, int> range, std::optional<int> value = std::nullopt)
		: super(wxID_ANY, "")
		, m_value(value.value_or(range.first))
		, m_range(range)
	{
	}

private:
	wxWindow* createWidget(
		wxWindow* parent,
		wxWindowID id,
		const std::string& str,
		const wxPoint& pos,
		const wxSize& size,
		long style) override
	{
		return new wxSlider(parent, id, m_value, m_range.first, m_range.second, pos, size, style);
	}

private:
	int m_value { 0 };
	std::pair<int, int> m_range;
};