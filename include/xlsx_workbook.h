#pragma once
#include <xlsx_worksheet.h>
#include <memory>
#include <xlsx_archive.h>
namespace xlsx_reader
{
	class workbook
	{
	public:
		std::vector<std::unique_ptr<worksheet>> _worksheets;
		std::vector<sheet_desc> sheet_relations; 
		std::vector<std::string_view> shared_string;
		const worksheet& get_worksheet(std::uint32_t sheet_idx) const;
		std::uint32_t get_worksheet_num() const;
		std::string_view workbook_name;
		std::string_view get_workbook_name() const;
		workbook(std::shared_ptr<archive> in_archive);
		friend std::ostream& operator<<(std::ostream& output_stream, const workbook& in_workbook);
	private:
		std::shared_ptr<archive> archive_content;
		
		worksheet* load_worksheet(std::string_view _input_string);
		const tinyxml2::XMLDocument* get_sheet_xml(std::uint32_t sheet_idx) const;
		friend class worksheet;

	};
}