#include "config.h"
#include "../shared/Tokenizer.h"
int main()
{
	Config conf;
	conf.ReadFile(_T("E:\\trinitycore\\src\\server\\worldserver\\worldserver.conf.dist"));
	//std::cin>>conf;
	std::_tstring m_logsDir = conf.Read(_T("LogsDir"),_T(""));
	if (!m_logsDir.empty())
		if ((m_logsDir.at(m_logsDir.length() - sizeof(_tchar)) != '/') && (m_logsDir.at(m_logsDir.length() -  sizeof(_tchar)) != '\\'))
			m_logsDir.push_back('/');
	std::_tistringstream ss(conf.Read(_T("Appenders"), _T("")));
	std::_tstring _name;
	do
	{
		ss >> _name;
		const _tchar* name=_name.c_str();
		if (!name || *name == '\0')
			return 0;

		// Format=type,level,flags,optional1,optional2
		// if type = File. optional1 = file and option2 = mode
		// if type = Console. optional1 = Color
		std::_tstring options = _T("Appender.");
		options.append(name);
		options =conf.Read(options.c_str(), _T(""));

		_tprintf(_T("Appenders.%s=%s\n"),options.c_str());
	}
	while (ss);

	std::_tcout<<conf;
	return 0;
}