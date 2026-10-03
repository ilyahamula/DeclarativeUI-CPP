#pragma once

// Custom composites: user-defined structs/classes, each standing for a group of
// widgets and used anywhere a built-in widget fits. DECLARE_UI(...)
// (include/declare_ui.hpp) writes their buildNode() and adds withFlags(),
// isDisabled() and isHidden(); its header comment has the rules of use.
//
// On show: an aggregate (PasswordRow), a class with a constructor and private
// data (LabeledField), composites of composites with a handler (LoginForm), and
// a composite whose root disables itself while an outer flag disables it too
// (ServerSettings) -- both flags apply.

#include "declarative_ui.hpp"

#include <string>
#include <utility>

// ---------------------------------------------------------------------------
// The demo's composites
// ---------------------------------------------------------------------------

namespace custom_composites
{
    constexpr int kLabelW = 80;
    constexpr int kRowH = 26;
    constexpr int kLabelH = 20;
}

// 1. The simplest form: an aggregate with a public reference member, the
//    Password row of drawControlsUI() lifted into a named type.
struct PasswordRow
{
    std::string& password;

    DECLARE_UI(HStack {
        StaticText{"Password:"}
            .withSize({custom_composites::kLabelW, custom_composites::kLabelH})
            .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 5)),
        PasswordInput{password}
            .withSize({-1, custom_composites::kRowH})
            .withFlags(LayoutFlags().Proportion(1))
            .withTooltip("At least 8 characters, one of them a digit.")
    })
};

// 2. A class with private data and a constructor: the label and placeholder are
//    copied in and kept by the composite; the text stays caller-owned and bound.
class LabeledField
{
public:
    LabeledField(std::string label, std::string& text, std::string placeholder = {})
        : m_label(std::move(label))
        , m_text(text)
        , m_placeholder(std::move(placeholder))
    {
    }

    DECLARE_UI(HStack {
        StaticText{m_label + ":"}
            .withSize({custom_composites::kLabelW, custom_composites::kLabelH})
            .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 5)),
        TextCtrl{m_text}
            .withPlaceholder(m_placeholder)
            .withSize({-1, custom_composites::kRowH})
            .withFlags(LayoutFlags().Proportion(1))
    })

private:
    std::string m_label;
    std::string& m_text;
    std::string m_placeholder;
};

// 3. A composite built from composites, with a handler. The handler captures the
//    caller's variables, never `this`: on wx/Qt the LoginForm is long gone by
//    the time the button is clicked.
class LoginForm
{
public:
    LoginForm(std::string& user, std::string& password, std::string& status)
        : m_user(user)
        , m_password(password)
        , m_status(status)
    {
    }

    DECLARE_UI(VStack {
        LabeledField{"User", m_user, "name@example.com"}
            .withFlags(LayoutFlags().Expand()),
        PasswordRow{m_password}
            .withFlags(LayoutFlags().Expand()),
        Button{"Sign in"}
            .withSize({110, 28})
            .isDefault()
            .withFlags(LayoutFlags().Border(Side::Top, 4))
            .onClick([&user = m_user, &password = m_password, &status = m_status] {
                status = password.size() >= 8
                    ? "Signed in as " + (user.empty() ? std::string("anonymous") : user)
                    : "Password too short";
            })
    })

private:
    std::string& m_user;
    std::string& m_password;
    std::string& m_status;
};

// 4. A composite whose root carries its OWN isDisabled(): the group is disabled
//    while "offline" is ticked. An outer .isDisabled(lockAll) on top of it is
//    the clash case -- both flags must still apply.
class ServerSettings
{
public:
    ServerSettings(std::string& host, int& port, bool& offline)
        : m_host(host)
        , m_port(port)
        , m_offline(offline)
    {
    }

    DECLARE_UI(VGroupBox { "Server",
        LayoutFlags().MinSize({340, 0}),
        LabeledField{"Host", m_host, "localhost"}
            .withFlags(LayoutFlags().Expand()),
        HStack {
            StaticText{"Port:"}
                .withSize({custom_composites::kLabelW, custom_composites::kLabelH})
                .withFlags(LayoutFlags().CenterVertical().Border(Side::Right, 5)),
            SpinBox{ { .min = 1, .max = 65535 }, m_port }
                .withSize({120, custom_composites::kRowH})
        }
    }.isDisabled(m_offline))

private:
    std::string& m_host;
    int& m_port;
    bool& m_offline;
};

// ---------------------------------------------------------------------------
// The dialog
// ---------------------------------------------------------------------------

struct DemoCustomComposites
{
    std::string user;
    std::string password;
    std::string status = "Not signed in";
    std::string host = "localhost";
    int port = 8080;
    bool lockAll = false;
    bool hideLogin = false;
    bool offline = false;
};

inline auto drawCustomCompositesUI(DemoCustomComposites& state)
{
    return Dialog {
        "Custom composites",
        VStack {
            LayoutFlags().Expand().Border(Side::All, 12),
            HStack {
                CheckBox{state.lockAll, "Lock all"}
                    .withTooltip("LoginForm{...}.isDisabled(lockAll) and ServerSettings{...}.isDisabled(lockAll)"),
                CheckBox{state.hideLogin, "Hide login"}
                    .withFlags(LayoutFlags().Border(Side::Left, 12))
                    .withTooltip("LoginForm{...}.isHidden(hideLogin)"),
                CheckBox{state.offline, "Offline"}
                    .withFlags(LayoutFlags().Border(Side::Left, 12))
                    .withTooltip("Disables the Server group from INSIDE the composite")
            },
            VGroupBox { "Login",
                LayoutFlags().MinSize({340, 0}).Border(Side::Top, 8),
                LoginForm{state.user, state.password, state.status}
                    .withFlags(LayoutFlags().Expand())
                    .isDisabled(state.lockAll)
                    .isHidden(state.hideLogin)
            },
            ServerSettings{state.host, state.port, state.offline}
                .withFlags(LayoutFlags().Expand().Border(Side::Top, 8))
                .isDisabled(state.lockAll),
            StaticText{state.status}
                .withSize({340, 20})
                .withFlags(LayoutFlags().Border(Side::Top, 10))
        }
    };
}
