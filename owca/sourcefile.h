#ifndef _RC_Y_SOURCE_FILE_H
#define _RC_Y_SOURCE_FILE_H

namespace owca {
	class owca_source_file;
	class owca_source_file_Text;
	class owca_source_file_Text_array;
	namespace __owca__ {
		class virtual_machine;
	}
}

namespace owca {

	class owca_source_file {
	public:
		virtual int next_line(const char *&src) const =0;
		virtual ~owca_source_file() { }
	};

	class owca_source_file_Text : public owca_source_file {
		mutable const char *txt;
		const char *end;
	public:
		owca_source_file_Text(const char *txt_, unsigned int len) : txt(txt_), end(txt_ + len) { }
		DLLEXPORT owca_source_file_Text(const char *txt_);
		DLLEXPORT int next_line(const char *&src) const;
	};

	class owca_source_file_Text_array : public owca_source_file {
		const char **txt;
		unsigned int cnt;
		mutable unsigned int index;
	public:
		DLLEXPORT owca_source_file_Text_array(const char **txt_, unsigned int cnt_=std::numeric_limits<unsigned int>::max());
		DLLEXPORT int next_line(const char *&src) const;
	};

}
#endif
