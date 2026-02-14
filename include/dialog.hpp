#pragma once

#include <memory>
#include <string>

#include "frameworks_core/DialogWrapper.hpp"
#include "fittable_layout.hpp"

template<FittableLayout Content>
struct Dialog
{
	Dialog(const std::string& title, Content content)
		: m_title(title)
		, m_content(std::move(content))
	{
	}

	Dialog(const std::string& title, Size size, Content content)
		: m_title(title)
		, m_size(size)
		, m_content(std::move(content))
	{
	}

	void show()
	{
		DialogWrapper wrapper(m_title, m_size);
		m_content.fitTo(&wrapper);
		wrapper.show();
	}

private:
	std::string m_title;
	Size m_size { -1, -1 };
	Content m_content;
};
