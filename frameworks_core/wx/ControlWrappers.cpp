#include "frameworks_core/ControlWrappers.hpp"
#include "frameworks_core/wx/DialogKeys.hpp"
#include "frameworks_core/wx/TextField.hpp"
#include "frameworks_core/wx/RefSync.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <memory>
#include <tuple>
#include <utility>

#ifdef USE_LOGGER
#include "Logger.hpp"
#endif

#include <wx/wx.h>
#include <wx/hyperlink.h>
#include <wx/srchctrl.h>
#include <wx/spinctrl.h>
#include <wx/datectrl.h>
#include <wx/timectrl.h>
#include <wx/dateevt.h>
#include <wx/tglbtn.h>
#include <wx/gauge.h>
#include <wx/statline.h>
#include <wx/clrpicker.h>
#include <wx/treectrl.h>
#include <wx/toolbar.h>
#include <wx/statusbr.h>
#include <wx/dataview.h>
#include <wx/settings.h>
#include <wx/collheaderctrl.h>
#include <wx/filepicker.h>
#include <wx/checklst.h>
#include <wx/timer.h>

#include "frameworks_core/wx/FileDialogSupport.hpp"
#include "frameworks_core/wx/Labels.hpp"
#include "frameworks_core/wx/RichTextPanel.hpp"

// Constructors only collect data and live inline in ControlWrappers.hpp.
// realize() creates the native wxWidget from the collected data (plus the
// position/size/style stored in the ControlWrapper base) under the given
// parent window and binds its events; the layout engine measures and places.

namespace
{

// wxImage can decode nothing until its handlers are registered, and the
// registration is process-wide -- so it is done once, by the first load.
void ensureImageHandlers()
{
	static const bool registered = [] {
		wxInitAllImageHandlers();
		return true;
	}();
	(void)registered;
}

// A path that fails to load is a caller's mistake we report through our own
// logger, never wx's: wxImage's wxLogError reaches wxLogGui, which shows it as
// a modal message box on the next idle -- in front of the user, or forever on
// a machine with nobody to dismiss it.
wxImage loadImage(const std::string& path)
{
	wxLogNull quiet;
	ensureImageHandlers();
	return wxImage(path, wxBITMAP_TYPE_ANY);
}

#ifdef __WXMSW__
// Win32 "clicks" a radio button that receives focus unchecked: its own
// WM_SETFOCUS sends BN_CLICKED unless the mouse holds capture, which is how
// arrow keys move the pick inside a native group. Ours are in no native group
// and each is a tab stop, so a dialog focusing its first control -- or Tab
// passing through -- would pick an option nobody chose and write it to the
// bound int. That one click is dropped. A mouse click (capture first, click on
// release) and Space (click on key-up) arrive outside WM_SETFOCUS as before.
class UngroupedRadioButton : public wxRadioButton
{
public:
	using wxRadioButton::wxRadioButton;

	WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) override
	{
		constexpr WXUINT kSetFocus = 0x0007; // WM_SETFOCUS, without <windows.h>
		if (message != kSetFocus)
			return wxRadioButton::MSWWindowProc(message, wParam, lParam);
		m_focusing = true;
		const WXLRESULT result = wxRadioButton::MSWWindowProc(message, wParam, lParam);
		m_focusing = false;
		return result;
	}

	bool MSWCommand(WXUINT param, WXWORD id) override
	{
		constexpr WXUINT kClicked = 0; // BN_CLICKED
		if (m_focusing && param == kClicked)
			return true;
		return wxRadioButton::MSWCommand(param, id);
	}

private:
	bool m_focusing = false;
};
#else
using UngroupedRadioButton = wxRadioButton;
#endif

} // unnamed namespace

// ButtonWrapper -----------------------------------------------------------

void ButtonWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ButtonWrapper::realize()\t-> new wxButton()\n");
#endif
	auto* btn = new wxButton(static_cast<wxWindow*>(parentWindow), wxID_ANY, wxLabelText(m_label),
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = btn;
	// Enter/Escape find the button through its mark; SetDefault() is only the
	// native look -- the window's char hook presses it before any port could.
	wx_dialog_keys::markButton(btn, m_dialogKeys);
	if (!m_iconPath.empty())
	{
		// Before the engine measures: the bitmap is part of the best size.
		wxImage image = loadImage(m_iconPath);
		if (image.IsOk())
		{
			image = image.Scale(std::max(1, m_iconSize.width), std::max(1, m_iconSize.height), wxIMAGE_QUALITY_HIGH);
			btn->SetBitmap(wxBitmap(image));
			btn->SetBitmapPosition(wxLEFT);
		}
#ifdef USE_LOGGER
		else
		{
			Logger::instance().log("ButtonWrapper::realize()\t-> icon \""
				+ m_iconPath + "\" failed to load; text only\n");
		}
#endif
	}
	if ((m_dialogKeys & kDefaultButton) != 0)
		btn->SetDefault();

	if (m_onClick)
		btn->Bind(wxEVT_BUTTON, [cb = std::move(m_onClick), nw = m_nativeWidget](wxCommandEvent&) { cb(nw); });

}

// TextCtrlWrapper -----------------------------------------------------------

namespace
{

// wxTextCtrl::SetHint, with the field's best size pinned across it. wx is the
// only backend whose measurement could notice a hint at all (ImGui never
// measures one and QLineEdit's sizeHint is a fixed character count), and a
// placeholder is usually longer than the text it stands in for -- so pinning
// first is what keeps an auto-fit dialog the same size on all three.
void applyHint(wxTextCtrl* txt, const std::string& hint)
{
	if (hint.empty())
		return;
	const wxSize best = txt->GetBestSize();
	txt->SetHint(hint);
	txt->CacheBestSize(best);
}

// onEnter: the field was created with wxTE_PROCESS_ENTER, so the window's char
// hook leaves Enter to it (DialogKeys.hpp) and this handler owns the whole
// sequence -- report, then press the default button, the order Qt's
// QLineEdit and ImGui produce. Not Skip()ped: what a skipped TEXT_ENTER does
// next differs by port.
void bindEnter(wxTextCtrl* txt, EventCallback<const std::string&> onEnter)
{
	if (!onEnter)
		return;
	txt->Bind(wxEVT_TEXT_ENTER, [txt, cb = std::move(onEnter)](wxCommandEvent&) {
		cb(txt->GetValue().ToStdString(), txt);
		wx_dialog_keys::press(txt, kDefaultButton);
	});
}

} // unnamed namespace

void TextCtrlWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("TextCtrlWrapper::realize()\t-> new wxTextCtrl()\n");
#endif
	const std::string& initial = m_value.get();
	auto* txt = new wxTextCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, initial,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height),
		m_style | (m_onEnter ? wxTE_PROCESS_ENTER : 0));
	applyHint(txt, m_placeholder);
	m_nativeWidget = txt;

	txt->Bind(wxEVT_TEXT, [commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxCommandEvent& evt) { commit(evt.GetString().ToStdString()); });
	bindEnter(txt, std::move(m_onEnter));
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(txt,
			[txt] { return txt->GetValue().ToStdString(); },
			[&value] { return value; },
			[txt](const std::string& v) { txt->ChangeValue(v); });
	}

	wx_text_field::apply(txt, std::move(m_field));
}

// PasswordInputWrapper -----------------------------------------------------------

void PasswordInputWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("PasswordInputWrapper::realize()\t-> new wxTextCtrl(wxTE_PASSWORD)\n");
#endif
	const std::string& initial = m_value.get();
	auto* txt = new wxTextCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, initial,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height),
		m_style | wxTE_PASSWORD | (m_onEnter ? wxTE_PROCESS_ENTER : 0));
	applyHint(txt, m_placeholder);
	m_nativeWidget = txt;

	txt->Bind(wxEVT_TEXT, [commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxCommandEvent& evt) { commit(evt.GetString().ToStdString()); });
	bindEnter(txt, std::move(m_onEnter));
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(txt,
			[txt] { return txt->GetValue().ToStdString(); },
			[&value] { return value; },
			[txt](const std::string& v) { txt->ChangeValue(v); });
	}

	wx_text_field::apply(txt, std::move(m_field));
}

// SearchFieldWrapper -----------------------------------------------------------

void SearchFieldWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("SearchFieldWrapper::realize()\t-> new wxSearchCtrl()\n");
#endif
	auto* search = new wxSearchCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, m_value.get(),
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxTE_PROCESS_ENTER);
	search->ShowSearchButton(true);
	search->ShowCancelButton(true);
	// The hint is pinned out of the best size, as applyHint() does for a
	// TextCtrl: a long placeholder must not widen an auto-fit window.
	if (!m_placeholder.empty())
	{
		const wxSize best = search->GetBestSize();
		search->SetDescriptiveText(m_placeholder);
		search->CacheBestSize(best);
	}
	m_nativeWidget = search;

	search->Bind(wxEVT_TEXT, [commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxCommandEvent& evt) {
		commit(evt.GetString().ToStdString());
	});
	// Enter (and, on macOS, a click on the magnifier). Not Skip()ped: the
	// window's char hook already left Enter to this control (focusWantsEnter),
	// and it must not reach a default button by any other route.
	if (m_onSearch)
	{
		search->Bind(wxEVT_SEARCH, [search, cb = std::move(m_onSearch)](wxCommandEvent&) {
			cb(search->GetValue().ToStdString(), search);
		});
	}
	// The cancel button: macOS empties the field itself, the generic control
	// only reports -- SetValue is the edit either way, and reaches onChange.
	search->Bind(wxEVT_SEARCH_CANCEL, [search](wxCommandEvent&) {
		if (!search->GetValue().empty())
			search->SetValue(wxString());
	});
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(search,
			[search] { return search->GetValue().ToStdString(); },
			[&value] { return value; },
			[search](const std::string& v) { search->ChangeValue(v); });
	}
}

// MultiLineTextCtrlWrapper -----------------------------------------------------------

void MultiLineTextCtrlWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("MultiLineTextCtrlWrapper::realize()\t-> new wxTextCtrl(wxTE_MULTILINE)\n");
#endif
	const std::string& initial = m_value.get();
	auto* txt = new wxTextCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, initial,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxTE_MULTILINE);
	m_nativeWidget = txt;

	txt->Bind(wxEVT_TEXT, [commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxCommandEvent& evt) { commit(evt.GetString().ToStdString()); });
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(txt,
			[txt] { return txt->GetValue().ToStdString(); },
			[&value] { return value; },
			[txt](const std::string& v) { txt->ChangeValue(v); });
	}

	wx_text_field::apply(txt, std::move(m_field));
}

// ReadonlyTextCtrlWrapper -----------------------------------------------------------

void ReadonlyTextCtrlWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ReadonlyTextCtrlWrapper::realize()\t-> new wxTextCtrl(wxTE_READONLY)\n");
#endif
	auto* txt = new wxTextCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, wxString::FromUTF8(m_value.get()),
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxTE_READONLY);
	m_nativeWidget = txt;

	// Bound: the field follows the caller's string. ChangeValue sends no
	// wxEVT_TEXT, and nothing listens for one anyway -- the user cannot type.
	if (const std::string* bound = m_value.boundValue())
	{
		bindExternalRefSync(txt,
			[txt] { return std::string(txt->GetValue().ToUTF8()); },
			[bound] { return *bound; },
			[txt](const std::string& v) { txt->ChangeValue(wxString::FromUTF8(v)); });
	}
}

// ClickableTextWrapper -----------------------------------------------------------

void ClickableTextWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ClickableTextWrapper::realize()\t-> new wxStaticText()\n");
#endif
	auto* st = new wxStaticText(static_cast<wxWindow*>(parentWindow), wxID_ANY, wxLabelText(m_text),
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = st;

	if (m_onClick)
		st->Bind(wxEVT_LEFT_DOWN, [cb = std::move(m_onClick), nw = m_nativeWidget](wxMouseEvent&) { cb(nw); });

}

// LinkTextWrapper -----------------------------------------------------------

void LinkTextWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("LinkTextWrapper::realize()\t-> new wxHyperlinkCtrl()\n");
#endif
	auto* link = new wxHyperlinkCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, m_text, wxEmptyString,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = link;

	if (m_onClick)
		link->Bind(wxEVT_HYPERLINK, [cb = std::move(m_onClick), nw = m_nativeWidget](wxHyperlinkEvent&) { cb(nw); });

}

// StaticTextWrapper -----------------------------------------------------------

void StaticTextWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("StaticTextWrapper::realize()\t-> new wxStaticText()\n");
#endif
	// wxST_NO_AUTORESIZE rides with the alignment bits and only with them: a
	// wxStaticText shrinks itself back to its text on every SetLabel otherwise,
	// and there would be no slack left in the frame to align in. A Left label
	// keeps exactly the style it always had.
	long align = 0;
	if (m_align == TextAlign::Center)
		align = wxALIGN_CENTRE_HORIZONTAL | wxST_NO_AUTORESIZE;
	else if (m_align == TextAlign::Right)
		align = wxALIGN_RIGHT | wxST_NO_AUTORESIZE;

	// A BOUND label never resizes itself on SetLabel -- the engine owns its
	// frame, which it keeps from the first text -- and ellipsizes a longer
	// text instead of spilling past that frame.
	if (m_text.isBound())
		align |= wxST_NO_AUTORESIZE | wxST_ELLIPSIZE_END;

	auto* label = new wxStaticText(static_cast<wxWindow*>(parentWindow), wxID_ANY, wxLabelText(m_text.get()),
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | align);
	m_nativeWidget = label;
	const wxSize best = label->GetBestSize();
	m_initialSize = Size { best.x, best.y };

	if (const std::string* bound = m_text.boundValue())
	{
		bindExternalRefSync(label,
			[label] { return std::string(label->GetLabelText().ToUTF8()); },
			[bound] { return *bound; },
			[label](const std::string& text) { label->SetLabel(wxLabelText(text)); });
	}
}

Size StaticTextWrapper::measureIntrinsic(const Constraints&)
{
	// Bound labels only (measuresItself): the size of the first text, never
	// the live one -- see StaticText.
	return m_initialSize;
}

// RichTextWrapper -----------------------------------------------------------

void RichTextWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("RichTextWrapper::realize()\t-> new RichTextPanel()\n");
#endif
	auto* panel = new RichTextPanel(static_cast<wxWindow*>(parentWindow), m_runs);
	m_nativeWidget = panel;

	if (m_onLink)
		panel->setOnLink([cb = std::move(m_onLink), nw = m_nativeWidget](const std::string& url) { cb(url, nw); });
}

Size RichTextWrapper::measureIntrinsic(const Constraints& c)
{
	const auto* panel = static_cast<const RichTextPanel*>(m_nativeWidget);
	if (panel == nullptr)
		return Size { 0, 0 };
	const RichTextLayout& layout = panel->layoutFor(wrapWidth(c));
	return Size { layout.width, layout.height };
}

// DatePickerWrapper -----------------------------------------------------------

void DatePickerWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("DatePickerWrapper::realize()\t-> new wxDatePickerCtrl()\n");
#endif
	const Date& dval = m_value.get();
	wxDateTime dt;
	dt.Set(static_cast<wxDateTime::wxDateTime_t>(dval.day),
		static_cast<wxDateTime::Month>(dval.month - 1),
		dval.year);
	auto* dp = new wxDatePickerCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, dt,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = dp;

	dp->Bind(wxEVT_DATE_CHANGED, [commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxDateEvent& evt) {
		const wxDateTime& d = evt.GetDate();
		Date date{ d.GetYear(), static_cast<int>(d.GetMonth()) + 1, d.GetDay() };
		commit(date);
	});
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		// Compare the Y/M/D triple, not the wxDateTime: the picker keeps a time-of-day
		// component that Date has no opinion about.
		bindExternalRefSync(dp,
			[dp] {
				const wxDateTime d = dp->GetValue();
				return std::tuple{ d.GetYear(), static_cast<int>(d.GetMonth()) + 1, static_cast<int>(d.GetDay()) };
			},
			[&value] { return std::tuple{ value.year, value.month, value.day }; },
			[dp](const std::tuple<int, int, int>& ymd) {
				wxDateTime d;
				d.Set(static_cast<wxDateTime::wxDateTime_t>(std::get<2>(ymd)),
					static_cast<wxDateTime::Month>(std::get<1>(ymd) - 1),
					std::get<0>(ymd));
				dp->SetValue(d);
			});
	}

}

// TimePickerWrapper -----------------------------------------------------------

void TimePickerWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("TimePickerWrapper::realize()\t-> new wxTimePickerCtrl()\n");
#endif
	const Time& tval = m_value.get();
	wxDateTime dt = wxDateTime::Now();
	dt.SetHour(tval.hour);
	dt.SetMinute(tval.minute);
	dt.SetSecond(tval.second);
	auto* tp = new wxTimePickerCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, dt,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = tp;

	tp->Bind(wxEVT_TIME_CHANGED, [commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxDateEvent& evt) {
		const wxDateTime& d = evt.GetDate();
		Time time{ d.GetHour(), d.GetMinute(), d.GetSecond() };
		commit(time);
	});
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(tp,
			[tp] {
				int h = 0, m = 0, s = 0;
				tp->GetTime(&h, &m, &s);
				return std::tuple{ h, m, s };
			},
			[&value] { return std::tuple{ value.hour, value.minute, value.second }; },
			[tp](const std::tuple<int, int, int>& hms) { tp->SetTime(std::get<0>(hms), std::get<1>(hms), std::get<2>(hms)); });
	}

}

// SliderWrapper -----------------------------------------------------------

template <SliderValue T>
void SliderWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("SliderWrapper::realize()\t-> new wxSlider()\n");
#endif
	const T& val = m_value.get();
	wxSlider* sl = nullptr;
	if constexpr (std::is_floating_point_v<T>)
	{
		int iMin = static_cast<int>(m_range.min / m_range.step);
		int iMax = static_cast<int>(m_range.max / m_range.step);
		int iVal = static_cast<int>(val / m_range.step);
		sl = new wxSlider(static_cast<wxWindow*>(parentWindow), wxID_ANY, iVal, iMin, iMax,
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	}
	else
	{
		sl = new wxSlider(static_cast<wxWindow*>(parentWindow), wxID_ANY, static_cast<int>(val), m_range.min, m_range.max,
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	}
	m_nativeWidget = sl;

	if constexpr (std::is_floating_point_v<T>)
		sl->Bind(wxEVT_SLIDER, [step = m_range.step, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxCommandEvent& evt) { commit(static_cast<T>(evt.GetInt()) * step); });
	else
		sl->Bind(wxEVT_SLIDER, [commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxCommandEvent& evt) { commit(static_cast<T>(evt.GetInt())); });
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		// wxSlider is integral; a float slider lives in step units, so compare there.
		bindExternalRefSync(sl,
			[sl] { return sl->GetValue(); },
			[&value, step = m_range.step] {
				if constexpr (std::is_floating_point_v<T>)
					return static_cast<int>(value / step);
				else
					return static_cast<int>(value);
			},
			[sl](int v) { sl->SetValue(v); });
	}

}

template class SliderWrapper<int>;
template class SliderWrapper<float>;

// SpinBoxWrapper -----------------------------------------------------------

template <SpinBoxValue T>
void SpinBoxWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("SpinBoxWrapper::realize()\t-> new wxSpinCtrl[Double]()\n");
#endif
	const T& val = m_value.get();
	if constexpr (std::is_same_v<T, int>)
	{
		auto* spin = new wxSpinCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY, wxEmptyString,
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style,
			m_range.min, m_range.max, static_cast<int>(val));
		m_nativeWidget = spin;

		spin->Bind(wxEVT_SPINCTRL, [commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxSpinEvent& evt) { commit(evt.GetInt()); });
		if (m_value.isBound())
		{
			auto& value = m_value.get();
			bindExternalRefSync(spin,
				[spin] { return spin->GetValue(); },
				[&value] { return static_cast<int>(value); },
				[spin](int v) { spin->SetValue(v); });
		}
	}
	else
	{
		auto* spin = new wxSpinCtrlDouble(static_cast<wxWindow*>(parentWindow), wxID_ANY, wxEmptyString,
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style,
			m_range.min, m_range.max, static_cast<double>(val), m_range.step);
		m_nativeWidget = spin;

		spin->Bind(wxEVT_SPINCTRLDOUBLE, [commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxSpinDoubleEvent& evt) { commit(static_cast<T>(evt.GetValue())); });
		if (m_value.isBound())
		{
			auto& value = m_value.get();
			// Quantise both sides to step units: wxSpinCtrlDouble rounds what it stores to
			// its display precision, so a raw double compare would push-and-round forever.
			bindExternalRefSync(spin,
				[spin, step = m_range.step] { return std::lround(spin->GetValue() / step); },
				[&value, step = m_range.step] { return std::lround(value / step); },
				[spin, step = m_range.step](long units) { spin->SetValue(static_cast<double>(units) * step); });
		}
	}

}

template class SpinBoxWrapper<int>;
template class SpinBoxWrapper<float>;

// RadioButtonWrapper -----------------------------------------------------------

template <RadioButtonValue T>
void RadioButtonWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("RadioButtonWrapper::realize()\t-> new wxRadioButton()\n");
#endif
	// This radio belongs to no native group. wx would otherwise chain it to
	// every radio created after the last wxRB_GROUP in the same parent -- and
	// every leaf here is parented flat to its dialog or page, so two groups
	// declared in one box would become one. The bound int is the group instead
	// (see RadioButtonWrapper), and the ref sync below unchecks the others.
	//
	// wxRB_SINGLE is the style that says so on MSW and GTK, but wxOSX ignores
	// it and chains the radio into its sibling's cycle anyway. There, wxRB_GROUP
	// on EVERY radio does the same job: each one starts a cycle nobody joins.
#ifdef __WXOSX__
	constexpr long kUngrouped = wxRB_GROUP;
#else
	constexpr long kUngrouped = wxRB_SINGLE;
#endif
	auto* rb = new UngroupedRadioButton(static_cast<wxWindow*>(parentWindow), wxID_ANY, wxLabelText(m_label),
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | kUngrouped);
	rb->SetValue(isChecked(m_value.get(), m_option));
	m_nativeWidget = rb;

	const T choice = picked(m_option);
	rb->Bind(wxEVT_RADIOBUTTON, [choice, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxCommandEvent&) { commit(choice); });
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		// Every radio on the int mirrors it: the one just picked is already
		// checked, and the rest see the int move away from their option and
		// uncheck themselves. SetValue sends no wxEVT_RADIOBUTTON.
		bindExternalRefSync(rb,
			[rb] { return rb->GetValue(); },
			[&value, option = m_option] { return isChecked(value, option); },
			[rb](bool on) { rb->SetValue(on); });
	}
}

template class RadioButtonWrapper<bool>;
template class RadioButtonWrapper<int>;

// CheckBoxWrapper -----------------------------------------------------------

void CheckBoxWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("CheckBoxWrapper::realize()\t-> new wxCheckBox()\n");
#endif
	const bool checked = m_value.get();
	auto* chk = new wxCheckBox(static_cast<wxWindow*>(parentWindow), wxID_ANY, wxLabelText(m_label),
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	chk->SetValue(checked);
	m_nativeWidget = chk;

	chk->Bind(wxEVT_CHECKBOX, [commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxCommandEvent& evt) { commit(evt.IsChecked()); });
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(chk,
			[chk] { return chk->GetValue(); },
			[&value] { return value; },
			[chk](bool on) { chk->SetValue(on); });
	}

}

// ToggleButtonWrapper -----------------------------------------------------------

void ToggleButtonWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ToggleButtonWrapper::realize()\t-> new wxToggleButton()\n");
#endif
	const bool toggled = m_value.get();
	auto* btn = new wxToggleButton(static_cast<wxWindow*>(parentWindow), wxID_ANY, wxLabelText(m_label),
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	btn->SetValue(toggled);
	m_nativeWidget = btn;

	btn->Bind(wxEVT_TOGGLEBUTTON, [commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxCommandEvent& evt) { commit(evt.IsChecked()); });
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(btn,
			[btn] { return btn->GetValue(); },
			[&value] { return value; },
			[btn](bool on) { btn->SetValue(on); });
	}

}

// ImageWrapper -----------------------------------------------------------

namespace
{
	// A wxStaticBitmap that remembers the picture it was given.
	//
	// It has to: a wxStaticBitmap draws its bitmap at its top-left and clips,
	// so it can do Stretch and nothing else -- every other mode needs the
	// picture COMPOSED against the frame first, and composing repeatedly from
	// an already-composed bitmap would lose everything the last crop threw away.
	//
	// There is deliberately no wxEVT_SIZE handler here. wxStaticBitmap does not
	// deliver one on wxOSX, so the frame arrives through ControlWrapper::placed()
	// instead -- which is the engine telling us directly, and therefore the same
	// moment on every wx port.
	// NOTE the leading "::" on every ScaleMode below. wxStaticBitmap has a
	// nested ScaleMode of its own (Scale_AspectFit and friends) which would
	// otherwise shadow ours -- and it is not a substitute: it has no Center,
	// and the point of R13.3 is that all three backends compute the same
	// rectangle from the same helper rather than each trusting its own native
	// idea of what "fit" means.
	class ScaledBitmapCtrl : public wxStaticBitmap
	{
	public:
		ScaledBitmapCtrl(wxWindow* parent, const wxImage& source,
			::ScaleMode mode, const wxPoint& pos, const wxSize& size, long style)
			: wxStaticBitmap(parent, wxID_ANY, wxBitmap(source), pos, size, style)
			, m_source(source)
			, m_mode(mode)
		{
			// The natural size is what the measure pass should read, and a
			// wxStaticBitmap derives it from whatever bitmap it holds -- which
			// stops being the original as soon as we compose. Pinned here, the
			// same move applyHint() makes across SetHint(). An explicit
			// withSize() still overrides it per axis in measureContent().
			CacheBestSize(wxSize(source.GetWidth(), source.GetHeight()));
		}

		// Guarded on the size it last composed at, so a relayout that did not
		// move this picture costs no scaling at all -- and SetBitmap's own
		// best-size update can never drive a second pass, since the composed
		// bitmap is exactly the size the control already has.
		void composeFor(const wxSize& box)
		{
			if (!m_source.IsOk() || box.x <= 0 || box.y <= 0 || box == m_composed)
				return;
			m_composed = box;

			const Rect target = scaledImageRect(m_mode,
				Size { m_source.GetWidth(), m_source.GetHeight() },
				Size { box.x, box.y });

			wxImage picture = (target.width == m_source.GetWidth()
					&& target.height == m_source.GetHeight())
				? m_source
				: m_source.Scale(target.width, target.height, wxIMAGE_QUALITY_HIGH);
			if (!picture.HasAlpha())
				picture.InitAlpha();   // opaque, so the paste below carries it over

			// Everything the mode does not cover stays transparent, so a Fit
			// letterbox shows the parent through it exactly as Qt's does and as
			// undrawn pixels do on ImGui.
			wxImage canvas(box.x, box.y);
			canvas.SetRGB(wxRect(0, 0, box.x, box.y), 0, 0, 0);
			canvas.InitAlpha();
			std::memset(canvas.GetAlpha(), wxIMAGE_ALPHA_TRANSPARENT,
				static_cast<size_t>(box.x) * static_cast<size_t>(box.y));

			// Paste clips on every side by itself, which IS the crop for Fill
			// and Center: the target rect is deliberately allowed to overflow.
			canvas.Paste(picture, target.x, target.y, wxIMAGE_ALPHA_BLEND_OVER);
			SetBitmap(wxBitmap(canvas));
		}

	private:
		wxImage m_source;
		::ScaleMode m_mode = ::ScaleMode::Stretch;
		wxSize m_composed { -1, -1 };
	};
}

void ImageWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ImageWrapper::realize()\t-> new wxStaticBitmap()\n");
#endif
	// The ORIGINAL picture goes in, unscaled: the scaling needs the frame, and
	// the frame is not decided until the engine places the control.
	wxImage source = loadImage(m_filePath);
	auto* bmpCtrl = new ScaledBitmapCtrl(static_cast<wxWindow*>(parentWindow),
		source.IsOk() ? source : wxImage(16, 16), m_scaleMode,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = bmpCtrl;

	if (m_onClick)
		bmpCtrl->Bind(wxEVT_LEFT_DOWN, [cb = std::move(m_onClick), nw = m_nativeWidget](wxMouseEvent&) { cb(nw); });

	if (m_onHover)
		bmpCtrl->Bind(wxEVT_ENTER_WINDOW, [cb = std::move(m_onHover), nw = m_nativeWidget](wxMouseEvent&) { cb(nw); });

}

void ImageWrapper::placed(const Rect& frame)
{
	if (auto* bmpCtrl = static_cast<ScaledBitmapCtrl*>(m_nativeWidget))
		bmpCtrl->composeFor(wxSize(frame.width, frame.height));
}

// ToolBarWrapper -----------------------------------------------------------

void ToolBarWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ToolBarWrapper::realize()\t-> new wxToolBar()\n");
#endif
	// A CHILD wxToolBar, deliberately not wxFrame::CreateToolBar(): that one
	// docks itself to a frame and would be invisible to the engine, and it
	// would make a toolbar impossible inside a Dialog or anywhere down a stack.
	long style = wxTB_HORIZONTAL | wxTB_FLAT | wxTB_NODIVIDER;
	const bool anyLabels = m_labelsForced
		|| std::any_of(m_tools.begin(), m_tools.end(), [](const ToolItem& tool) {
			return !tool.isSeparator && tool.iconPath.empty();
		});
	if (anyLabels)
		style |= wxTB_TEXT;

	auto* bar = new wxToolBar(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), style | m_style);
	bar->SetToolBitmapSize(wxSize(m_iconSize.width, m_iconSize.height));
	m_nativeWidget = bar;

	for (ToolItem& tool : m_tools)
	{
		if (tool.isSeparator)
		{
			bar->AddSeparator();
			continue;
		}

		// A tool with no usable icon shows its label instead, so the row is
		// never blank -- and a bitmap is still required by AddTool, so an empty
		// one of the right size stands in.
		wxBitmap bitmap;
		if (!tool.iconPath.empty())
		{
			wxImage image = loadImage(tool.iconPath);
			if (image.IsOk())
			{
				if (m_iconSize.width > 0 && m_iconSize.height > 0)
					image = image.Scale(m_iconSize.width, m_iconSize.height, wxIMAGE_QUALITY_HIGH);
				bitmap = wxBitmap(image);
			}
#ifdef USE_LOGGER
			else
			{
				Logger::instance().log("ToolBarWrapper::realize()\t-> icon \""
					+ tool.iconPath + "\" failed to load; falling back to the label\n");
			}
#endif
		}
		if (!bitmap.IsOk())
		{
			bitmap = wxBitmap(std::max(1, m_iconSize.width), std::max(1, m_iconSize.height));
			// Fully transparent, so only the label reads.
			wxImage blank = bitmap.ConvertToImage();
			blank.InitAlpha();
			std::memset(blank.GetAlpha(), 0,
				(std::size_t)blank.GetWidth() * (std::size_t)blank.GetHeight());
			bitmap = wxBitmap(blank);
		}

		const int id = wxWindow::NewControlId();
		const wxString label = wxString::FromUTF8(tool.label);
		const wxString help = wxString::FromUTF8(tool.tooltip);
		ToolItem* model = &tool;
		if (model->toggledFlag)
		{
			bar->AddCheckTool(id, label, bitmap, wxNullBitmap, help);
			bar->ToggleTool(id, model->toggledFlag->get());
		}
		else
		{
			bar->AddTool(id, label, bitmap, help);
		}
		bar->EnableTool(id, !model->disabledFlag.get());

		bar->Bind(wxEVT_TOOL, [model, bar, id](wxCommandEvent& event) {
			// Value first, then the callback, so a handler reading the bound
			// bool sees the state the user just produced.
			if (model->toggledFlag)
				model->toggledFlag->set(bar->GetToolState(id));
			if (model->clickHandler)
				model->clickHandler();
			event.Skip(false);
		}, id);

		// Bound flags are polled, never pushed on disagreement -- the shape
		// every externally-written value in the framework uses.
		if (model->toggledFlag && model->toggledFlag->isBound())
		{
			const bool* flag = model->toggledFlag->boundValue();
			bindExternalRefSync(bar,
				[bar, id] { return bar->GetToolState(id); },
				[flag] { return *flag; },
				[bar, id](bool value) { bar->ToggleTool(id, value); });
		}
		if (model->disabledFlag.isBound())
		{
			const bool* flag = model->disabledFlag.boundValue();
			bindExternalRefSync(bar,
				[bar, id] { return bar->GetToolEnabled(id); },
				[flag] { return !*flag; },
				[bar, id](bool enabled) { bar->EnableTool(id, enabled); });
		}
	}

	// Lays the tools out and fixes the bar's best size -- nothing appears
	// without it.
	bar->Realize();
}

// StatusBarWrapper -----------------------------------------------------------

void StatusBarWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("StatusBarWrapper::realize()\t-> new wxStatusBar()\n");
#endif
	// A CHILD wxStatusBar, deliberately not wxFrame::CreateStatusBar(): that one
	// docks itself to a frame, out of the engine's sight, and would make a
	// status bar impossible in a Dialog or anywhere else down a stack.
	// wxSTB_SHOW_TIPS (a truncated field's text as its tip) and a tooltip of
	// our own are mutually exclusive -- wx asserts on SetToolTip -- so the
	// caller's withTooltip() wins when there is one.
	long style = wxSTB_DEFAULT_STYLE | m_style;
	if (!tooltip().empty() || boundTooltip() != nullptr)
		style &= ~wxSTB_SHOW_TIPS;
	auto* bar = new wxStatusBar(static_cast<wxWindow*>(parentWindow), wxID_ANY, style);
	m_nativeWidget = bar;

	const int count = m_fields.empty() ? 1 : (int)m_fields.size();
	bar->SetFieldsCount(count);

	// wx reads a NEGATIVE width as a stretch weight and a positive one as fixed
	// pixels, which is exactly the split StatusField already describes.
	std::vector<int> widths;
	widths.reserve((std::size_t)count);
	for (const StatusField& field : m_fields)
		widths.push_back(field.width > 0 ? field.width : -1);
	if (m_fields.empty())
		widths.push_back(-1);
	bar->SetStatusWidths(count, widths.data());

	for (std::size_t index = 0; index < m_fields.size(); ++index)
	{
		StatusField& field = m_fields[index];
		bar->SetStatusText(wxString::FromUTF8(field.text.get()), (int)index);

		// A bound field is the whole point: the text is written from elsewhere
		// and wx has no notification for that, so it is polled like any other
		// external ref.
		if (field.text.isBound())
		{
			const std::string* bound = field.text.boundValue();
			const int pane = (int)index;
			bindExternalRefSync(bar,
				[bar, pane] { return std::string(bar->GetStatusText(pane).ToUTF8()); },
				[bound] { return *bound; },
				[bar, pane](const std::string& text) {
					bar->SetStatusText(wxString::FromUTF8(text), pane);
				});
		}
	}
}

Size StatusBarWrapper::measureIntrinsic(const Constraints&)
{
	const auto* bar = static_cast<const wxStatusBar*>(m_nativeWidget);
	return Size { statusBarContentWidth(m_fields), bar != nullptr ? bar->GetBestSize().y : 0 };
}

// ColorPickerWrapper -----------------------------------------------------------

void ColorPickerWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ColorPickerWrapper::realize()\t-> new wxColourPickerCtrl()\n");
#endif
	const Color& cval = m_value.get();
	auto* picker = new wxColourPickerCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxColour(static_cast<unsigned char>(cval.r * 255),
		         static_cast<unsigned char>(cval.g * 255),
		         static_cast<unsigned char>(cval.b * 255),
		         static_cast<unsigned char>(cval.a * 255)),
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = picker;

	picker->Bind(wxEVT_COLOURPICKER_CHANGED, [commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxColourPickerEvent& evt) {
		const wxColour& c = evt.GetColour();
		commit(Color{ c.Red() / 255.0f, c.Green() / 255.0f, c.Blue() / 255.0f, c.Alpha() / 255.0f });
	});
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		// Compare as wxColour: Color is float 0..1 but the control quantises to 0..255,
		// so the byte domain is the only one where round-tripping is stable.
		bindExternalRefSync(picker,
			[picker] { return picker->GetColour(); },
			[&value] {
				return wxColour(static_cast<unsigned char>(value.r * 255),
					static_cast<unsigned char>(value.g * 255),
					static_cast<unsigned char>(value.b * 255),
					static_cast<unsigned char>(value.a * 255));
			},
			[picker](const wxColour& c) { picker->SetColour(c); });
	}

}

// FilePickerWrapper -----------------------------------------------------------

void FilePickerWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("FilePickerWrapper::realize()\t-> new wxFilePickerCtrl()/wxDirPickerCtrl()\n");
#endif
	auto* parent = static_cast<wxWindow*>(parentWindow);
	const wxString initial = wxString::FromUTF8(m_value.get());
	const wxString message = wxString::FromUTF8(m_dialogTitle);

	// USE_TEXTCTRL on both: it is what makes the typed path a first-class way
	// of setting the value (R11.4). wx raises the same CHANGED event for a
	// typed edit as for a pick, so one handler serves both.
	if (m_mode == FileMode::Directory)
	{
		auto* picker = new wxDirPickerCtrl(parent, wxID_ANY, initial,
			message.empty() ? wxString(wxDirSelectorPromptStr) : message,
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height),
			m_style | wxDIRP_USE_TEXTCTRL | wxDIRP_DIR_MUST_EXIST);
		m_nativeWidget = picker;
	}
	else
	{
		const long modeStyle = m_mode == FileMode::Save
			? (wxFLP_SAVE | wxFLP_OVERWRITE_PROMPT)
			: (wxFLP_OPEN | wxFLP_FILE_MUST_EXIST);
		auto* picker = new wxFilePickerCtrl(parent, wxID_ANY, initial,
			message.empty() ? wxString(wxFileSelectorPromptStr) : message,
			wxWildcardFor(m_filters),
			wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height),
			m_style | modeStyle | wxFLP_USE_TEXTCTRL);
		m_nativeWidget = picker;
	}

	// wxFilePickerCtrl and wxDirPickerCtrl share a base with GetPath/SetPath and
	// they raise the same wxFileDirPickerEvent, so everything below is written
	// once against the base -- the branch above is the only place the two modes
	// differ. Both event types are bound because the base does not say which of
	// them this instance will send.
	auto* picker = static_cast<wxFileDirPickerCtrlBase*>(m_nativeWidget);
	auto bindChanged = [picker](std::function<void(const wxString&)> handler) {
		picker->Bind(wxEVT_FILEPICKER_CHANGED,
			[h = handler](wxFileDirPickerEvent& evt) { h(evt.GetPath()); });
		picker->Bind(wxEVT_DIRPICKER_CHANGED,
			[h = std::move(handler)](wxFileDirPickerEvent& evt) { h(evt.GetPath()); });
	};

	bindChanged([commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](const wxString& path) {
		commit(std::string(path.ToUTF8()));
	});
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		// SetPath does not raise the CHANGED event, so mirroring an external
		// write never re-enters the handler above.
		bindExternalRefSync(picker,
			[picker] { return std::string(picker->GetPath().ToUTF8()); },
			[&value] { return value; },
			[picker](const std::string& v) { picker->SetPath(wxString::FromUTF8(v)); });
	}

}

// SeparatorWrapper -----------------------------------------------------------

void SeparatorWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("SeparatorWrapper::realize()\t-> new wxStaticLine()\n");
#endif
	const long orientStyle = m_orient == Orientation::Vertical ? wxLI_VERTICAL : wxLI_HORIZONTAL;
	m_nativeWidget = new wxStaticLine(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | orientStyle);

}

// SplitterSashWrapper -----------------------------------------------------------

void SplitterSashWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("SplitterSashWrapper::realize()\t-> new wxPanel()\n");
#endif
	const bool horizontal = m_state->orientation == Orientation::Horizontal;

	auto* sash = new wxPanel(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style);
	m_nativeWidget = sash;
	sash->SetCursor(wxCursor(horizontal ? wxCURSOR_SIZEWE : wxCURSOR_SIZENS));
	sash->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW));
	// An empty wxPanel's best size is arbitrary; the sash is exactly this thick
	// on its own axis and asks for nothing on the other, where the engine
	// stretches it across both panes.
	sash->SetMinSize(wxSize(horizontal ? SplitterState::kSashThickness : 0,
		horizontal ? 0 : SplitterState::kSashThickness));

	// Per-drag state, shared by the handlers below through a shared_ptr rather
	// than through the wrapper: every handler dies with the panel, and nothing
	// here may capture the wrapper (wx/RefSync.hpp -- wrapper and window
	// teardown order is not fixed). The SplitterState may be: it lives in the
	// node tree, which the engine session owns.
	struct SashDrag
	{
		int anchorScreen = 0;
		int anchorPos = 0;
		bool active = false;
	};
	auto drag = std::make_shared<SashDrag>();
	SplitterState* state = m_state;

	// The drag anchors on the position arrange RESOLVED when the gesture began,
	// so a long drag cannot accumulate rounding the way a per-event delta would,
	// and screen coordinates are used because the panel itself moves underneath
	// the pointer as the layout follows it.
	sash->Bind(wxEVT_LEFT_DOWN, [sash, state, drag, horizontal](wxMouseEvent& event) {
		const wxPoint screen = sash->ClientToScreen(event.GetPosition());
		drag->anchorScreen = horizontal ? screen.x : screen.y;
		drag->anchorPos = state->resolved;
		drag->active = true;
		if (!sash->HasCapture())
			sash->CaptureMouse();
		event.Skip();
	});
	sash->Bind(wxEVT_MOTION, [sash, state, drag, horizontal](wxMouseEvent& event) {
		if (drag->active && event.Dragging())
		{
			const wxPoint screen = sash->ClientToScreen(event.GetPosition());
			const int moved = (horizontal ? screen.x : screen.y) - drag->anchorScreen;
			state->position.set(std::clamp(drag->anchorPos + moved,
				state->lowerBound, state->upperBound));
		}
		event.Skip();
	});
	sash->Bind(wxEVT_LEFT_UP, [sash, drag](wxMouseEvent& event) {
		drag->active = false;
		if (sash->HasCapture())
			sash->ReleaseMouse();
		event.Skip();
	});
	// The capture is already gone here; releasing it again would assert.
	sash->Bind(wxEVT_MOUSE_CAPTURE_LOST, [drag](wxMouseCaptureLostEvent&) {
		drag->active = false;
	});

}

// ExpanderHeaderWrapper -----------------------------------------------------------

void ExpanderHeaderWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ExpanderHeaderWrapper::realize()\t-> new wxCollapsibleHeaderCtrl()\n");
#endif
	// wxCollapsibleHeaderCtrl is the header half of wxCollapsiblePane without
	// the pane -- which is precisely what is wanted here, since the content is
	// an engine-arranged subtree rather than something wx may size for us.
	auto* header = new wxCollapsibleHeaderCtrl(static_cast<wxWindow*>(parentWindow),
		wxID_ANY, wxLabelText(m_label), wxPoint(m_pos.x, m_pos.y),
		wxSize(m_size.width, m_size.height), m_style);
	header->SetCollapsed(!m_state->expanded.get());
	m_nativeWidget = header;

	// The ExpanderState lives in the node tree, which the engine session owns
	// and which outlives every window here; the wrapper must never be captured
	// (wx/RefSync.hpp -- wrapper and window teardown order is not fixed).
	ExpanderState* state = m_state;
	header->Bind(wxEVT_COLLAPSIBLEHEADER_CHANGED, [header, state](wxCommandEvent& event) {
		state->expanded.set(!header->IsCollapsed());
		event.Skip();
	});

	// A bound flag can be written from anywhere, and the header control applies
	// its own state only when clicked -- so the arrow is mirrored like any other
	// external ref. The relayout that follows is armed separately, by the
	// session's poll: this only keeps the header itself honest.
	if (m_state->expanded.isBound())
	{
		bindExternalRefSync(header,
			[header] { return !header->IsCollapsed(); },
			[state] { return state->expanded.get(); },
			[header](bool open) { header->SetCollapsed(!open); });
	}

}

// ProgressBarWrapper -----------------------------------------------------------

// How often an indeterminate gauge is pulsed. wxGTK advances the marquee one
// pulse step per call, so this is the animation's frame rate there; wxOSX and
// wxMSW switch the native control into a self-animating mode on the first call
// and ignore the rest. 100 ms is a full sweep per second on GTK and costs
// nothing on the two ports that do not need it.
static constexpr int kGaugePulseIntervalMs = 100;

void ProgressBarWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ProgressBarWrapper::realize()\t-> new wxGauge()\n");
#endif
	// The bound float is a 0..100 percentage, matching the gauge's own integer range.
	const auto toGauge = [](float v) { return static_cast<int>(std::clamp(v, 0.0f, 100.0f)); };

	const float initial = m_value.get();
	auto* gauge = new wxGauge(static_cast<wxWindow*>(parentWindow), wxID_ANY, 100,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), m_style | wxGA_HORIZONTAL | wxGA_SMOOTH);
	m_nativeWidget = gauge;

	if (m_indeterminate)
	{
		// Busy mode. wx is the one backend that will not animate by itself
		// everywhere, so the pulse needs a clock -- and the idle sync every other
		// binding rides on is the wrong one: it fires when the event queue drains
		// and deliberately never asks for more, so an untouched window would
		// freeze the animation outright. A timer keeps ticking while nothing
		// happens.
		//
		// The gauge owns the timer through the handler's capture: the dynamic
		// event table dies with the window, which drops the last reference and
		// stops the timer. Nothing here outlives the gauge.
		auto timer = std::make_shared<wxTimer>(gauge);
		gauge->Bind(wxEVT_TIMER, [gauge, timer](wxTimerEvent&) { gauge->Pulse(); });
		gauge->Pulse();
		timer->Start(kGaugePulseIntervalMs);
		return;
	}

	gauge->SetValue(toGauge(initial));

	// A progress bar has no input events of its own -- the bound float is only ever
	// written from outside -- so the idle sync is the whole story here.
	if (m_value.isBound())
	{
		bindExternalRefSync(gauge,
			[gauge] { return gauge->GetValue(); },
			[&value = m_value.get(), toGauge] { return toGauge(value); },
			[gauge](int v) { gauge->SetValue(v); });
	}
}

namespace
{

// The strings a wx item container holds now, and the one call that replaces
// them. wxComboBox, wxListBox and wxCheckListBox all derive from
// wxItemContainer, so one pair serves the three. Set() sends no selection event.
ItemList nativeItems(const wxItemContainerImmutable* container)
{
	ItemList items;
	items.reserve(container->GetCount());
	for (unsigned int i = 0; i < container->GetCount(); ++i)
		items.emplace_back(container->GetString(i).ToUTF8());
	return items;
}

wxArrayString toArrayString(const ItemList& items)
{
	wxArrayString out;
	for (const auto& item : items)
		out.Add(wxString::FromUTF8(item));
	return out;
}

} // unnamed namespace

// ComboBoxWrapper -----------------------------------------------------------

template <ComboBoxValue T>
void ComboBoxWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ComboBoxWrapper::realize()\t-> new wxComboBox()\n");
#endif
	// wxCB_READONLY: a pick from the list and nothing else, as QComboBox and
	// ImGui::Combo are. An editable wxComboBox would let the user type text no
	// wxEVT_COMBOBOX ever reports and that an int binding cannot represent.
	// Selection goes through the index setters, which send no event, so the
	// ref syncs below never re-enter the handler.
	auto* combo = new wxComboBox(static_cast<wxWindow*>(parentWindow), wxID_ANY, "",
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height),
		toArrayString(m_choices.get()), m_style | wxCB_READONLY);
	const auto select = [combo](const T& value) {
		if constexpr (std::is_same_v<T, std::string>)
			combo->SetSelection(combo->FindString(wxString::FromUTF8(value), true));
		else
			combo->SetSelection(value >= 0 && value < (int)combo->GetCount() ? value : wxNOT_FOUND);
	};
	const auto current = [combo]() -> T {
		if constexpr (std::is_same_v<T, std::string>)
			return std::string(combo->GetStringSelection().ToUTF8());
		else
			return combo->GetSelection();
	};
	select(m_value.get());
	m_nativeWidget = combo;
	const wxSize best = combo->GetBestSize();
	m_initialSize = Size { best.x, best.y };

	combo->Bind(wxEVT_COMBOBOX, [current, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxCommandEvent&) {
		commit(current());
	});

	// Bound choices: repopulate when the caller's vector changes, keeping the
	// selection by value -- the bound one when there is one, else whatever was
	// picked. Registered before the selection sync, so a tick that changes
	// both sees the new list first.
	if (const ItemList* boundItems = m_choices.boundValue())
	{
		const T* boundValue = m_value.boundValue();
		bindWatchedRefSync(combo, watchRefs(boundItems),
			[combo] { return nativeItems(combo); },
			[boundItems] { return *boundItems; },
			[combo, select, current, boundValue](const ItemList& items) {
				const T keep = boundValue != nullptr ? *boundValue : current();
				combo->Set(toArrayString(items));
				select(keep);
			});
	}
	if (const T* boundValue = m_value.boundValue())
	{
		bindExternalRefSync(combo,
			current,
			[boundValue] { return *boundValue; },
			select);
	}
}

template <ComboBoxValue T>
Size ComboBoxWrapper<T>::measureIntrinsic(const Constraints&)
{
	return m_initialSize; // bound choices only (measuresItself)
}

template class ComboBoxWrapper<std::string>;
template class ComboBoxWrapper<int>;

// EditableComboWrapper -----------------------------------------------------------

void EditableComboWrapper::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("EditableComboWrapper::realize()\t-> new wxComboBox()\n");
#endif
	// No wxCB_READONLY: the text is the value. Measured before the hint, which
	// must not widen an auto-fit window (applyHint's rule).
	auto* combo = new wxComboBox(static_cast<wxWindow*>(parentWindow), wxID_ANY, m_value.get(),
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height),
		toArrayString(m_items.get()), m_style);
	const wxSize best = combo->GetBestSize();
	m_initialSize = Size { best.x, best.y };
	if (!m_placeholder.empty())
	{
		combo->SetHint(m_placeholder);
		combo->CacheBestSize(best);
	}
	m_nativeWidget = combo;

	// Typing raises wxEVT_TEXT; a pick raises wxEVT_COMBOBOX and, on most ports
	// but not all, a wxEVT_TEXT as well. Both report through one commit that
	// drops a repeat of the text it last saw, so a pick is one onChange
	// everywhere. The RefSync push records what it wrote for the same reason.
	auto last = std::make_shared<std::string>(m_value.get());
	auto report = [combo, last, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](const std::string& text) {
		if (text == *last)
			return;
		*last = text;
		commit(text);
	};
	combo->Bind(wxEVT_TEXT, [report](wxCommandEvent& evt) { report(evt.GetString().ToStdString()); });
	combo->Bind(wxEVT_COMBOBOX, [combo, report](wxCommandEvent&) { report(combo->GetStringSelection().ToStdString()); });

	// Bound items: repopulate, keeping the text -- Set() clears it on some ports.
	if (const ItemList* boundItems = m_items.boundValue())
	{
		bindWatchedRefSync(combo, watchRefs(boundItems),
			[combo] { return nativeItems(combo); },
			[boundItems] { return *boundItems; },
			[combo](const ItemList& items) {
				const wxString keep = combo->GetValue();
				combo->Set(toArrayString(items));
				combo->ChangeValue(keep);
			});
	}
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindExternalRefSync(combo,
			[combo] { return combo->GetValue().ToStdString(); },
			[&value] { return value; },
			[combo, last](const std::string& v) {
				*last = v;
				combo->ChangeValue(v);
			});
	}
}

Size EditableComboWrapper::measureIntrinsic(const Constraints&)
{
	return m_initialSize; // bound items only (measuresItself)
}

// ListBoxWrapper -----------------------------------------------------------

namespace
{

// wx spells "no selection" as wxNOT_FOUND for single-select boxes and as an
// empty selection array for multi-select ones; both arrive here as an empty
// index list.
std::vector<int> listBoxSelection(const wxListBox* list, bool multiSelect)
{
	std::vector<int> indices;
	if (multiSelect)
	{
		wxArrayInt selections;
		list->GetSelections(selections);
		indices.reserve(selections.GetCount());
		for (int index : selections)
			indices.push_back(index);
	}
	else if (const int selection = list->GetSelection(); selection != wxNOT_FOUND)
	{
		indices.push_back(selection);
	}
	return indices;
}

// Programmatic selection: wx setters do not fire wxEVT_LISTBOX, so this never
// re-enters the user's onChange -- which is what the ref sync needs of a push.
void setListBoxSelection(wxListBox* list, const std::vector<int>& indices, bool multiSelect)
{
	if (!multiSelect)
	{
		list->SetSelection(indices.empty() ? wxNOT_FOUND : indices.front());
		return;
	}

	for (unsigned int i = 0; i < list->GetCount(); ++i)
	{
		if (std::find(indices.begin(), indices.end(), static_cast<int>(i)) != indices.end())
			list->SetSelection(i);
		else
			list->Deselect(i);
	}
}

} // unnamed namespace

template <ListBoxValue T>
void ListBoxWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("ListBoxWrapper::realize()\t-> new wxListBox()\n");
#endif
	// wxLB_EXTENDED gives ctrl/shift-click range selection; wxLB_MULTIPLE would
	// toggle on a plain click, which is not what a desktop list does.
	const long selectionStyle = kMultiSelect ? wxLB_EXTENDED : wxLB_SINGLE;
	auto* list = new wxListBox(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), toArrayString(m_items.get()),
		m_style | selectionStyle | wxLB_NEEDED_SB);
	setListBoxSelection(list, indicesFor(m_items.get(), boundValue()), kMultiSelect);
	m_nativeWidget = list;

	// wxListBox's own best height grows with the item count, so a long list would
	// ask the engine for a window taller than the screen. Pin it to visibleRows --
	// the same height the Qt and ImGui wrappers compute -- and keep wx's
	// content-derived best width.
	constexpr int kListBoxFrame = 6; // border the native box draws around its rows
	const wxSize best = list->GetBestSize();
	const int rowHeight = list->GetCharHeight() + 2;
	list->CacheBestSize(wxSize(best.x, rowHeight * m_visibleRows + kListBoxFrame));
	m_initialSize = Size { best.x, rowHeight * m_visibleRows + kListBoxFrame };

	const ItemsView items(m_items);
	list->Bind(wxEVT_LISTBOX, [list, items, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxCommandEvent&) {
		commit(valueFor(items(), listBoxSelection(list, kMultiSelect)));
	});

	// Bound items: repopulate, keeping the selection by value (see ComboBox).
	if (const ItemList* boundItems = items.bound())
	{
		const T* boundValue = m_value.boundValue();
		bindWatchedRefSync(list, watchRefs(boundItems),
			[list] { return nativeItems(list); },
			[boundItems] { return *boundItems; },
			[list, boundValue](const ItemList& next) {
				const T keep = boundValue != nullptr
					? *boundValue
					: valueFor(nativeItems(list), listBoxSelection(list, kMultiSelect));
				list->Set(toArrayString(next));
				setListBoxSelection(list, indicesFor(next, keep), kMultiSelect);
			});
	}
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindWatchedRefSync(list, watchRefs(&value, items.bound()),
			[list] { return listBoxSelection(list, kMultiSelect); },
			[&value, items] { return indicesFor(items(), value); },
			[list](const std::vector<int>& indices) { setListBoxSelection(list, indices, kMultiSelect); });
	}
}

template <ListBoxValue T>
Size ListBoxWrapper<T>::measureIntrinsic(const Constraints&)
{
	return m_initialSize; // bound items only (measuresItself)
}

template class ListBoxWrapper<int>;
template class ListBoxWrapper<std::string>;
template class ListBoxWrapper<std::vector<int>>;
template class ListBoxWrapper<std::vector<std::string>>;

// CheckListBoxWrapper -----------------------------------------------------------

namespace
{

std::vector<int> checkListChecked(const wxCheckListBox* list)
{
	std::vector<int> indices;
	for (unsigned int i = 0; i < list->GetCount(); ++i)
	{
		if (list->IsChecked(i))
			indices.push_back(static_cast<int>(i));
	}
	return indices;
}

// Programmatic ticking: wxCheckListBox::Check() does not fire
// wxEVT_CHECKLISTBOX, so this never re-enters the user's onChange -- which is
// what the ref sync needs of a push.
void setCheckListChecked(wxCheckListBox* list, const std::vector<int>& indices)
{
	for (unsigned int i = 0; i < list->GetCount(); ++i)
		list->Check(i, std::find(indices.begin(), indices.end(), static_cast<int>(i)) != indices.end());
}

} // unnamed namespace

template <CheckListValue T>
void CheckListBoxWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("CheckListBoxWrapper::realize()\t-> new wxCheckListBox()\n");
#endif
	// Single-SELECTION, whatever the checked set holds: the highlight and the
	// ticks are independent, and a multi-selection highlight would only suggest
	// otherwise.
	auto* list = new wxCheckListBox(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height), toArrayString(m_items.get()),
		m_style | wxLB_SINGLE | wxLB_NEEDED_SB);
	setCheckListChecked(list, indicesFor(m_items.get(), boundValue()));
	m_nativeWidget = list;

	// Same reason ListBoxWrapper pins its height: the native best size grows
	// with the item count, so a long list would ask for a window taller than
	// the screen. visibleRows is what makes the three backends agree.
	constexpr int kListBoxFrame = 6; // border the native box draws around its rows
	const wxSize best = list->GetBestSize();
	const int rowHeight = list->GetCharHeight() + 2;
	list->CacheBestSize(wxSize(best.x, rowHeight * m_visibleRows + kListBoxFrame));
	m_initialSize = Size { best.x, rowHeight * m_visibleRows + kListBoxFrame };

	const ItemsView items(m_items);
	list->Bind(wxEVT_CHECKLISTBOX, [list, items, commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxCommandEvent&) {
		commit(valueFor(items(), checkListChecked(list)));
	});

	// Bound items: repopulate, keeping the ticks by value (see ComboBox).
	if (const ItemList* boundItems = items.bound())
	{
		const T* boundValue = m_value.boundValue();
		bindWatchedRefSync(list, watchRefs(boundItems),
			[list] { return nativeItems(list); },
			[boundItems] { return *boundItems; },
			[list, boundValue](const ItemList& next) {
				const T keep = boundValue != nullptr
					? *boundValue
					: valueFor(nativeItems(list), checkListChecked(list));
				list->Set(toArrayString(next));
				setCheckListChecked(list, indicesFor(next, keep));
			});
	}
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindWatchedRefSync(list, watchRefs(&value, items.bound()),
			[list] { return checkListChecked(list); },
			[&value, items] { return indicesFor(items(), value); },
			[list](const std::vector<int>& indices) { setCheckListChecked(list, indices); });
	}
}

template <CheckListValue T>
Size CheckListBoxWrapper<T>::measureIntrinsic(const Constraints&)
{
	return m_initialSize; // bound items only (measuresItself)
}

template class CheckListBoxWrapper<std::vector<int>>;
template class CheckListBoxWrapper<std::vector<std::string>>;

// TreeViewWrapper -----------------------------------------------------------

namespace
{

// Carries an item's full path on the native item, so a selection event decodes
// to a path with a single lookup instead of walking back up through labels --
// which would also be ambiguous once two siblings share a name.
class TreePathData : public wxTreeItemData
{
public:
	explicit TreePathData(std::string itemPath)
		: path(std::move(itemPath))
	{
	}

	std::string path;
};

std::string treeItemPath(const wxTreeCtrl* tree, const wxTreeItemId& id)
{
	if (!id.IsOk())
		return {};
	const auto* data = static_cast<TreePathData*>(tree->GetItemData(id));
	return data ? data->path : std::string {};
}

// Depth-first over every item below `parent`. The hidden root is never visited
// with itself as an argument, so it never appears in a selection.
void forEachTreeItem(wxTreeCtrl* tree, const wxTreeItemId& parent,
	const std::function<void(const wxTreeItemId&)>& fn)
{
	wxTreeItemIdValue cookie;
	for (wxTreeItemId id = tree->GetFirstChild(parent, cookie); id.IsOk();
		id = tree->GetNextChild(parent, cookie))
	{
		fn(id);
		forEachTreeItem(tree, id, fn);
	}
}

std::vector<std::string> treeSelection(wxTreeCtrl* tree, bool multiSelect)
{
	std::vector<std::string> paths;
	if (multiSelect)
	{
		wxArrayTreeItemIds ids;
		tree->GetSelections(ids);
		paths.reserve(ids.GetCount());
		for (const auto& id : ids)
			paths.push_back(treeItemPath(tree, id));
	}
	else if (const wxTreeItemId id = tree->GetSelection(); id.IsOk())
	{
		paths.push_back(treeItemPath(tree, id));
	}
	// The hidden root carries no TreePathData and wx can report it selected;
	// an empty path is that non-item, not a selection.
	paths.erase(std::remove(paths.begin(), paths.end(), std::string {}), paths.end());
	return paths;
}

// Programmatic selection. Unlike the list-box setters, wxTreeCtrl::SelectItem
// DOES fire wxEVT_TREE_SEL_CHANGED, so every caller here has to hold the
// re-entry guard the ref sync installs -- see realize().
void setTreeSelection(wxTreeCtrl* tree, const std::vector<std::string>& paths, bool multiSelect)
{
	if (multiSelect)
		tree->UnselectAll();
	else
		tree->Unselect();

	if (paths.empty())
		return;

	forEachTreeItem(tree, tree->GetRootItem(), [&](const wxTreeItemId& id) {
		const std::string path = treeItemPath(tree, id);
		if (multiSelect
			? std::find(paths.begin(), paths.end(), path) != paths.end()
			: path == paths.front())
		{
			tree->SelectItem(id);
		}
	});
}

void addTreeItems(wxTreeCtrl* tree, const wxTreeItemId& parent,
	const std::vector<TreeItem>& items, const std::string& parentPath, char separator)
{
	for (const TreeItem& item : items)
	{
		const std::string path = parentPath.empty()
			? item.label
			: parentPath + separator + item.label;
		const wxTreeItemId id = tree->AppendItem(parent, item.label, -1, -1, new TreePathData(path));
		addTreeItems(tree, id, item.children, path, separator);
		// Expanding is done after the children exist: wx has nothing to expand
		// on a childless item and silently ignores the call.
		if (item.expanded && !item.children.empty())
			tree->Expand(id);
	}
}

// Every item path in the tree, and the ones currently open -- what a refill
// needs to keep the user's open/closed state (TreeViewWrapper::openAfterRefill).
void treeExpansion(wxTreeCtrl* tree, std::vector<std::string>& all, std::vector<std::string>& open)
{
	forEachTreeItem(tree, tree->GetRootItem(), [&](const wxTreeItemId& id) {
		const std::string path = treeItemPath(tree, id);
		all.push_back(path);
		if (tree->IsExpanded(id))
			open.push_back(path);
	});
}

void applyTreeExpansion(wxTreeCtrl* tree, const std::vector<std::string>& open)
{
	forEachTreeItem(tree, tree->GetRootItem(), [&](const wxTreeItemId& id) {
		if (!tree->ItemHasChildren(id))
			return;
		if (std::find(open.begin(), open.end(), treeItemPath(tree, id)) != open.end())
			tree->Expand(id);
		else
			tree->Collapse(id);
	});
}

} // unnamed namespace

template <TreeViewValue T>
void TreeViewWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("TreeViewWrapper::realize()\t-> new wxTreeCtrl()\n");
#endif
	// wxTreeCtrl has exactly one root, but a TreeItem list has many top-level
	// items, so the root is a hidden placeholder every top-level item hangs off.
	const long selectionStyle = m_multiSelect ? wxTR_MULTIPLE : wxTR_SINGLE;
	auto* tree = new wxTreeCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height),
		m_style | selectionStyle | wxTR_HIDE_ROOT | wxTR_HAS_BUTTONS | wxTR_NO_LINES);
	tree->AddRoot("");
	addTreeItems(tree, tree->GetRootItem(), m_items.get(), std::string {}, kPathSeparator);
	m_nativeWidget = tree;

	// wxTreeCtrl's best size is its client area, not its content, so it would
	// ask the engine for whatever it happens to have been given. Compute the
	// same shape the other two backends do: widest indented label, visibleRows
	// of height.
	constexpr int kTreeFrame = 6;      // border the native control draws
	constexpr int kScrollbarSlack = 20; // room for the vertical scrollbar
	int widest = 0;
	forEachItem(m_items.get(), [&](const TreeItem& item, const std::string&, int depth) {
		widest = std::max(widest,
			static_cast<int>(tree->GetIndent()) * (depth + 1)
				+ tree->GetTextExtent(item.label).GetWidth());
	});
	const int rowHeight = tree->GetCharHeight() + 4;
	tree->CacheBestSize(wxSize(widest + kTreeFrame + kScrollbarSlack,
		rowHeight * m_visibleRows + kTreeFrame));
	m_initialSize = Size { widest + kTreeFrame + kScrollbarSlack, rowHeight * m_visibleRows + kTreeFrame };

	setTreeSelection(tree, pathsFor(boundValue()), m_multiSelect);

	// SelectItem notifies, so mirroring the ref into the control would re-enter
	// this handler and echo the value straight back out. The guard is shared by
	// both lambdas and lives as long as they do.
	auto syncing = std::make_shared<bool>(false);
	tree->Bind(wxEVT_TREE_SEL_CHANGED, [tree, syncing, multi = m_multiSelect,
		commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxTreeEvent& evt) {
		evt.Skip();
		// wxGTK reports the selection emptying as the window is torn down;
		// that is no pick of the user's, and the caller's value must keep it.
		if (!*syncing && !tree->IsBeingDeleted())
			commit(valueFor(treeSelection(tree, multi)));
	});

	// Bound items: refill when the caller's tree changes, registered BEFORE the
	// selection sync. The selection is kept by path (the bound one, else what
	// was picked) and every item that is still there keeps the open/closed
	// state the user left it at. The tree keeps its first size (measuresItself).
	if (const std::vector<TreeItem>* boundItems = m_items.boundValue())
	{
		auto shown = std::make_shared<std::vector<TreeItem>>(*boundItems);
		const T* boundSelection = m_value.boundValue();
		bindWatchedRefSync(tree, watchRefs(boundItems),
			[shown] { return *shown; },
			[boundItems] { return *boundItems; },
			[tree, shown, syncing, boundSelection, multi = m_multiSelect](const std::vector<TreeItem>& next) {
				const std::vector<std::string> keep = boundSelection != nullptr
					? pathsFor(*boundSelection)
					: treeSelection(tree, multi);
				std::vector<std::string> before;
				std::vector<std::string> openBefore;
				treeExpansion(tree, before, openBefore);
				*syncing = true;
				tree->DeleteChildren(tree->GetRootItem());
				addTreeItems(tree, tree->GetRootItem(), next, std::string {}, kPathSeparator);
				applyTreeExpansion(tree, openAfterRefill(next, before, openBefore));
				setTreeSelection(tree, keep, multi);
				*syncing = false;
				*shown = next;
			});
	}
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindWatchedRefSync(tree, watchRefs(&value),
			[tree, multi = m_multiSelect] { return treeSelection(tree, multi); },
			[&value] { return pathsFor(value); },
			[tree, syncing, multi = m_multiSelect](const std::vector<std::string>& paths) {
				*syncing = true;
				setTreeSelection(tree, paths, multi);
				*syncing = false;
			});
	}
}

template <TreeViewValue T>
Size TreeViewWrapper<T>::measureIntrinsic(const Constraints&)
{
	return m_initialSize; // bound items only (measuresItself)
}

template class TreeViewWrapper<std::string>;
template class TreeViewWrapper<std::vector<std::string>>;

// TableWrapper -----------------------------------------------------------

namespace
{

// wxDataViewListCtrl rather than wxListCtrl or wxGrid. wxListCtrl in report
// mode can only edit its first column (wxLC_EDIT_LABELS), which rules it out
// the moment a Table has an editable column further right; wxGrid is a
// spreadsheet, with row headers and a cell-range selection model that looks
// nothing like QTableWidget or an ImGui table. wxDataViewListCtrl is the one
// wx control with per-column editability, per-column sorting and row
// selection -- the same three axes the other two backends offer.

// An item's ORIGINAL row index, which wx carries for us in the item data. It
// stays with the row when a column sort reorders the view, so it survives
// exactly what a view position does not.
//
// GetItemData() indexes the store with GetRow(item) unchecked, and on wxGTK
// (where the store maps items through a hash) an item it no longer holds is
// wxNOT_FOUND -- an out-of-range read. So the row is checked first.
int dataViewRowIndex(wxDataViewListCtrl* view, const wxDataViewItem& item)
{
	const wxDataViewListStore* store = view->GetStore();
	if (!item.IsOk() || store->GetRow(item) >= static_cast<unsigned>(store->GetItemCount()))
		return -1;
	return static_cast<int>(view->GetItemData(item));
}

std::vector<int> dataViewSelection(wxDataViewListCtrl* view, bool multiSelect)
{
	std::vector<int> indices;
	if (multiSelect)
	{
		wxDataViewItemArray items;
		view->GetSelections(items);
		indices.reserve(items.GetCount());
		for (const auto& item : items)
		{
			if (const int row = dataViewRowIndex(view, item); row >= 0)
				indices.push_back(row);
		}
	}
	else if (const int row = dataViewRowIndex(view, view->GetSelection()); row >= 0)
	{
		indices.push_back(row);
	}
	// Reported in original-index order, not the order wx happens to hold the
	// selection in, so a multi-select binding reads the same on all three
	// backends however the view is currently sorted.
	std::sort(indices.begin(), indices.end());
	return indices;
}

// The rows the control holds, by ORIGINAL index -- whatever order a sort put
// them in on screen.
TableRows dataViewRows(wxDataViewListCtrl* view, int columnCount)
{
	const int count = static_cast<int>(view->GetItemCount());
	TableRows rows(static_cast<std::size_t>(count), TableRow(static_cast<std::size_t>(columnCount)));
	for (int viewRow = 0; viewRow < count; ++viewRow)
	{
		const int row = dataViewRowIndex(view, view->RowToItem(viewRow));
		if (row < 0 || row >= count)
			continue;
		for (int column = 0; column < columnCount; ++column)
			rows[row][column] = view->GetTextValue(viewRow, column).ToStdString();
	}
	return rows;
}

// Replace every row, each carrying its original index as item data.
void fillDataView(wxDataViewListCtrl* view, const TableRows& rows, int columnCount)
{
	view->DeleteAllItems();
	for (int row = 0; row < static_cast<int>(rows.size()); ++row)
	{
		wxVector<wxVariant> values;
		values.reserve(static_cast<std::size_t>(columnCount));
		for (int column = 0; column < columnCount; ++column)
			values.push_back(wxVariant(wxString(TableWrapper<int>::cellText(rows, row, column))));
		view->AppendItem(values, static_cast<wxUIntPtr>(row));
	}
}

// Programmatic selection. wx sends wxEVT_DATAVIEW_SELECTION_CHANGED for these
// on the platforms whose native control notifies on selection (as wxTreeCtrl
// does, and unlike the list-box setters), so every caller here has to hold the
// re-entry guard the ref sync installs -- see realize().
void setDataViewSelection(wxDataViewListCtrl* view, const std::vector<int>& indices, bool multiSelect)
{
	view->UnselectAll();
	if (indices.empty())
		return;

	wxDataViewItemArray items;
	for (int viewRow = 0; viewRow < static_cast<int>(view->GetItemCount()); ++viewRow)
	{
		const wxDataViewItem item = view->RowToItem(viewRow);
		const int row = dataViewRowIndex(view, item);
		if (row < 0)
			continue;
		if (multiSelect
			? std::find(indices.begin(), indices.end(), row) != indices.end()
			: row == indices.front())
		{
			items.Add(item);
		}
	}

	if (multiSelect)
		view->SetSelections(items);
	else if (!items.IsEmpty())
		view->Select(items[0]);
}

} // unnamed namespace

template <TableValue T>
void TableWrapper<T>::realize(void* parentWindow)
{
#ifdef USE_LOGGER
	Logger::instance().log("TableWrapper::realize()\t-> new wxDataViewListCtrl()\n");
#endif
	// wxDV_MULTIPLE is wx's ctrl/shift-click mode; wxDV_SINGLE is the default
	// and is spelled out here so the two read as a pair.
	const long selectionStyle = kMultiSelect ? wxDV_MULTIPLE : wxDV_SINGLE;
	auto* view = new wxDataViewListCtrl(static_cast<wxWindow*>(parentWindow), wxID_ANY,
		wxPoint(m_pos.x, m_pos.y), wxSize(m_size.width, m_size.height),
		m_style | selectionStyle | wxDV_ROW_LINES);
	m_nativeWidget = view;

	const TableRows& rows = m_rows.get();
	const auto measureText = [view](const std::string& text) {
		return view->GetTextExtent(text).GetWidth();
	};

	// Columns are given the width the shared policy computes rather than wx's
	// wxCOL_WIDTH_DEFAULT, so a measured column comes out the same width here as
	// it does on Qt and ImGui.
	constexpr int kCellPadding = 12; // wx draws the cell's text inset on both sides
	int contentWidth = 0;
	for (int column = 0; column < static_cast<int>(m_columns.size()); ++column)
	{
		const TableColumn& spec = m_columns[column];
		const int width = columnWidth(m_columns, rows, column, measureText) + kCellPadding;
		contentWidth += width;
		view->AppendTextColumn(spec.label,
			spec.editable ? wxDATAVIEW_CELL_EDITABLE : wxDATAVIEW_CELL_INERT,
			width, wxALIGN_LEFT,
			wxDATAVIEW_COL_RESIZABLE | (spec.sortable ? wxDATAVIEW_COL_SORTABLE : 0));
	}

	// The original index rides along as item data -- see dataViewRowIndex().
	const int columnCount = static_cast<int>(m_columns.size());
	fillDataView(view, rows, columnCount);

	// wxDataViewListCtrl's best size is its client area rather than its content,
	// so it would ask the engine for whatever it happens to have been given.
	// Compute the same shape the other two backends do: summed column widths,
	// a header plus visibleRows of body.
	constexpr int kViewFrame = 6;       // border the native control draws
	constexpr int kScrollbarSlack = 20; // room for the vertical scrollbar
	const int rowHeight = view->GetCharHeight() + 6;
	view->CacheBestSize(wxSize(contentWidth + kViewFrame + kScrollbarSlack,
		rowHeight * (m_visibleRows + 1) + kViewFrame));

	setDataViewSelection(view, rowIndicesFor(rows, boundValue()), kMultiSelect);

	// Rows the handlers read: the caller's own while bound, so an edit made
	// anywhere is visible here, and a snapshot otherwise. Never the wrapper's --
	// wrapper and window teardown order is not fixed (see wx/RefSync.hpp), and
	// the snapshot is shared rather than copied into each lambda.
	TableRows* editTarget = boundRows();
	auto snapshot = std::make_shared<const TableRows>(rows);
	const auto liveRows = [editTarget, snapshot]() -> const TableRows& {
		return editTarget ? *editTarget : *snapshot;
	};

	if (std::any_of(m_columns.begin(), m_columns.end(),
		[](const TableColumn& column) { return column.editable; }))
	{
		view->Bind(wxEVT_DATAVIEW_ITEM_EDITING_DONE, [view, editTarget,
			cb = std::move(m_onCellChange)](wxDataViewEvent& evt) {
			evt.Skip();
			// Escape leaves the cell untouched, and wx says so here rather than
			// by withholding the event.
			if (evt.IsEditCancelled())
				return;
			const int row = dataViewRowIndex(view, evt.GetItem());
			if (row < 0)
				return;
			const std::string text = evt.GetValue().GetString().ToStdString();
			applyCellEdit(editTarget, row, evt.GetColumn(), text);
			if (cb)
				cb(row, evt.GetColumn(), text);
		});
	}

	// The selection setters notify, so mirroring the ref into the control would
	// re-enter this handler and echo the value straight back out. The guard is
	// shared by both lambdas and lives as long as they do.
	auto syncing = std::make_shared<bool>(false);
	view->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, [view, syncing, liveRows,
		commit = commitTo(m_value, std::move(m_onChange), m_nativeWidget)](wxDataViewEvent& evt) {
		evt.Skip();
		// As for the tree: a teardown's selection change is not the user's.
		if (!*syncing && !view->IsBeingDeleted())
			commit(valueFor(liveRows(), dataViewSelection(view, kMultiSelect)));
	});

	// Bound rows: refill when the caller's data changes -- registered BEFORE the
	// selection sync, which then reads indices against the new rows. Compared in
	// the control's own shape, so a cell edit (already written through above,
	// and already on screen) is no change. The columns keep the widths the
	// first rows gave them, as a bound list keeps its first width, the sort is
	// re-applied, and the selection is kept by value.
	if (editTarget != nullptr)
	{
		const T* boundSelection = m_value.boundValue();
		bindWatchedRefSync(view, watchRefs(editTarget),
			[view, columnCount] { return dataViewRows(view, columnCount); },
			[editTarget, columnCount] { return normalizedRows(*editTarget, columnCount); },
			[view, syncing, boundSelection, columnCount](const TableRows& next) {
				const T keep = boundSelection != nullptr
					? *boundSelection
					: valueFor(dataViewRows(view, columnCount), dataViewSelection(view, kMultiSelect));
				*syncing = true;
				fillDataView(view, next, columnCount);
				// Only a sort the user picked is re-applied: with no sorting
				// column wxGTK still sorts, by column -1, reading past the row.
				if (wxDataViewModel* model = view->GetModel(); model && view->GetSortingColumn())
					model->Resort();
				setDataViewSelection(view, rowIndicesFor(next, keep), kMultiSelect);
				*syncing = false;
			});
	}
	if (m_value.isBound())
	{
		auto& value = m_value.get();
		bindWatchedRefSync(view, watchRefs(&value, editTarget),
			[view] { return dataViewSelection(view, kMultiSelect); },
			[&value, liveRows] { return rowIndicesFor(liveRows(), value); },
			[view, syncing](const std::vector<int>& indices) {
				*syncing = true;
				setDataViewSelection(view, indices, kMultiSelect);
				*syncing = false;
			});
	}
}

template class TableWrapper<int>;
template class TableWrapper<std::string>;
template class TableWrapper<std::vector<int>>;
template class TableWrapper<std::vector<std::string>>;
