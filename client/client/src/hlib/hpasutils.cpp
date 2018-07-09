
//============================================================================
// Name : pasutils.c
// Author : hibiki
// Version : 0.1
// Copyright : nosense
// Description : old units
//============================================================================

#ifndef PASUTILS_C_
#define PASUTILS_C_

#include "hpasutils.hpp"

extern void IPCSendMsg(string data);

static int SigNum = 0;
static sigset_t SigSet;
static int pid = 0;
static string LastError = "";
static bool H_INITED = false;
static list<string> Params;

uint64_t DiskFree(const string path)
{
	struct statfs diskInfo;
	if (statfs(path.c_str(), &diskInfo) < 0) return -1;
	unsigned long long blocksize = diskInfo.f_bsize;
	unsigned long long totalsize = blocksize * diskInfo.f_blocks;
	return totalsize;
}

uint64_t DiskAvail(const string path)
{
	struct statfs diskInfo;
	if (statfs(path.c_str(), &diskInfo) < 0) return -1;
	unsigned long long blocksize = diskInfo.f_bsize;
	unsigned long long totalsize = blocksize * diskInfo.f_bavail;
	return totalsize;
}

uint64_t DiskTotal(const string path)
{
	struct statfs diskInfo;
	if (statfs(path.c_str(), &diskInfo) < 0) return -1;
	unsigned long long blocksize = diskInfo.f_bsize;
	unsigned long long totalsize = blocksize * diskInfo.f_blocks;
	return totalsize;
}

double UnixToDateTime(const Int64 AValue)
{
	return ((double)AValue) / SecsPerDay + UnixDateDelta;
}

double GetFileDate(const string filename)
{
	struct stat fileInfo;
	if (stat(filename.c_str(), &fileInfo) < 0) return 0;
	return UnixToDateTime(fileInfo.st_mtim.tv_sec);
}

int64_t GetFileSize(const string filename)
{
	struct stat fileInfo;
	if (stat(filename.c_str(), &fileInfo) < 0) return -1;
	return fileInfo.st_size;
}

string IntToStr(const int n)
{
  ostringstream result;
  result << n;
  return result.str();
}

string UIntToStr(const uint n)
{
  ostringstream result;
  result << n;
  return result.str();
}

string Int64ToStr(const long long n)
{
  ostringstream result;
  result << n;
  return result.str();
}

string UInt64ToStr(const unsigned long long n)
{
  ostringstream result;
  result << n;
  return result.str();
}

string AppName() // need delete
{
  string e = IntToStr(getpid());
  e = string("/proc/") + e + string("/exe");
  char* fullPath = new char [256];
  memset(fullPath, 0, 256);
  readlink(e.c_str(), fullPath, 256);
  e = string(fullPath);
  free(fullPath);
  return e;
}

string ThisPath()
{
  return ExtractFilepath(AppName());
}

string ExtractFilename(const string ExeName)
{
	char *ch = strdup(ExeName.c_str());
	string e = string(basename(ch));
	free(ch);
	return e;
}

string ExtractFilepath(const string ExeName)
{
	char *ch = strdup(ExeName.c_str());
	string e = string(dirname(ch));
	free(ch);
	if ((Length(e) > 0) && (RightStr(e, 1) != "/")) e += "/";
	return e;
}

bool WriteInstallConfig(const string ExeName, const string Intro)
{
	string ServiceName = ExtractFilename(ExeName);
	string ShellScript = "/etc/init.d/" + ServiceName;
	FILE * t;
  	t = fopen(ShellScript.c_str(), "w+");
  	if (t == NULL) return false;
  	fprintf(t, "#\n");
	#ifdef H_REDHAT_STYLE
  	fprintf(t, "# %s Start/Stop the %s daemon.\n", ServiceName.c_str(), ServiceName.c_str());
  	fprintf(t, "#\n");
  	fprintf(t, "# chkconfig: 2345 99 01\n");
  	fprintf(t, "# description: %s\n", Intro.c_str());
  	fprintf(t, "# processname: %s\n", ServiceName.c_str());
  	fprintf(t, "# pidfile: /var/run/%s.pid\n", ServiceName.c_str());
  	fprintf(t, "#\n");
  	fprintf(t, "# Source function library.\n");
  	fprintf(t, ". /etc/init.d/functions\n");
    #endif
	#ifdef H_DEBIAN_STYLE
  	fprintf(t, "### BEGIN INIT INFO\n");
  	fprintf(t, "# Provides:          %s\n", ServiceName.c_str());
  	fprintf(t, "# Required-Start:    $local_fs	$remote_fs\n");
  	fprintf(t, "# Required-Stop:     $local_fs	$remote_fs\n");
  	fprintf(t, "# Default-Start:     2 3 4 5\n");
  	fprintf(t, "# Default-Stop:      0 1 6\n");
  	fprintf(t, "# Short-Description: Start/stop %s\n", ServiceName.c_str());
  	fprintf(t, "# Description:       %s\n", Intro.c_str());
  	fprintf(t, "### END INIT INFO\n");
	#endif
  	fprintf(t, "\n");
  	// try modify languages
  	fprintf(t, "export LANG=\"zh_CN.UTF-8\"\n");
  	fprintf(t, "export LC_ALL=\"zh_CN.UTF-8\"\n");
  	// try end
  	fprintf(t, "case $1 in\n");
  	fprintf(t, "    start)\n");
  	fprintf(t, "    echo -n \"Starting %s ...\"\n", ServiceName.c_str());
  	fprintf(t, "    %s\n", ExeName.c_str());
  	fprintf(t, "    echo\n");
    #ifdef H_REDHAT_STYLE
  	fprintf(t, "    touch /var/lock/subsys/%s\n", ServiceName.c_str());
	#endif
  	fprintf(t, "    ;;\n");
  	fprintf(t, "    stop)\n");
  	fprintf(t, "    echo -n \"Shuting down %s ...\"\n", ServiceName.c_str());
  	fprintf(t, "    %s -stop\n", ExeName.c_str());
  	fprintf(t, "    echo\n");
	#ifdef H_REDHAT_STYLE
  	fprintf(t, "    rm -f /var/lock/subsys/%s\n", ServiceName.c_str());
	#endif
  	fprintf(t, "    rm -f /var/run/%s.pid\n", ServiceName.c_str());
  	fprintf(t, "    ;;\n");
  	fprintf(t, "    restart)\n");
  	fprintf(t, "    $0 stop\n");
  	fprintf(t, "    $0 start\n");
  	fprintf(t, "    ;;\n");
  	fprintf(t, "    reload)\n");
  	fprintf(t, "    echo -n \"Reloading %s:\"\n", ServiceName.c_str());
  	fprintf(t, "    killproc %s -HUP\n", ServiceName.c_str());
  	fprintf(t, "    echo\n");
  	fprintf(t, "    ;;\n");
  	fprintf(t, "    *)\n");
  	fprintf(t, "    echo \"Usage: $(basename $0) start|stop|restart|reload\"\n");
  	fprintf(t, "    exit 1\n");
  	fprintf(t, "esac\n");
  	fprintf(t, "exit 0\n");
    chmod(ShellScript.c_str(), 755);
  	fsync(fileno(t));
  	fclose(t);
  	return true;
}

bool Install(string intro)
{
	string ExeName = AppName();
	string ServiceName;

	if (!WriteInstallConfig(ExeName, intro))
	{
		return false;
	}

  	ServiceName = ExtractFilename(ExeName);
    #ifdef H_REDHAT_STYLE
    string Shortcut = string("chkconfig --add ") + ServiceName;
	#endif
	#ifdef H_DEBIAN_STYLE
    string Shortcut = string("sudo update-rc.d ") + ServiceName + " defaults";
	#endif
    return system(Shortcut.c_str()) == 0;
}

bool Uninstall()
{
	string ExeName = AppName();
	string ServiceName = ExtractFilename(ExeName);
    #ifdef H_REDHAT_STYLE
    string Shortcut = string("chkconfig --del ") + ServiceName;
	#endif
	#ifdef H_DEBIAN_STYLE
    string Shortcut = string("sudo update-rc.d -f ") + ServiceName + " remove";
	#endif
    return system(Shortcut.c_str()) == 0;
}

bool RegisterService(const string s)
{
	string filename = string("/var/run/") + s + string(".pid");
	FILE * f;
	f = fopen(filename.c_str(), "w+");
	if (f == NULL) return false;
    fprintf(f, "%d", getpid());
    fclose(f);
    return true;
}

bool FileExists(const string filename)
{
	FILE * fp = fopen(filename.c_str(), "r");
	if (!fp) return false;
	fclose(fp);
	return true;
}

int GetServicePID(const string t)
{
	string s = string("/var/run/") + t + string(".pid");
	FILE * f;
	f = fopen(s.c_str(), "r");
	if (f == NULL) return false;
    int ret = -1;
    fscanf(f, "%d", &ret);
    fclose(f);
    return ret;
}

void TranslateSig(int sig)
{
	SigNum = sig;
}

void Initialization()
{
	if (H_INITED) return;

	string e = IntToStr(getpid());
	e = string("/proc/") + e + "/cmdline";
	FILE * f = fopen(e.c_str(), "r");
	if (!f) return;
	stringstream ss;
	int ch;
	while ((ch=fgetc(f))!=EOF)
		if (!ch)
		{
			Params.push_back(ss.str());
			ss.str("");
		}
		else
			ss.put(char(ch));
	fclose(f);

	H_INITED = true;
}

uint ParamCount()
{
	if (!H_INITED) Initialization();
	return Params.size() - 1;
}

string ParamStr(const uint index)
{
	if (!H_INITED) Initialization();
	if ((index < 0) || (index >= Params.size())) return NULL;
	list<string>::iterator itl = Params.begin();
	for(int i = 0; i < (int)index; i++) itl++;
	return (string)*itl;
}

string GetCommandLine()
{
	string e = IntToStr(getpid());
	e = string("/proc/") + e + string("/cmdline");
	FILE * f = fopen(e.c_str(), "r");
	if (f == NULL)
	{
		e = "";
		return e;
	}
	stringstream ss;
	int ch;
	while ((ch=fgetc(f))!=EOF) ss.put(char(ch==0?32:ch));
	e = string(ss.str());
	fclose(f);
	return e;
}

static void RunComdLine()
{
	printf("Please input param\n");
	stringstream ss;
	Params.clear();
	while(1)
	{
		char ch = 0;
		ss.str("");
		int i = 1;
		while((ch = getc(stdin)) != 10)
		{
			ss.put(char(ch));
			if(ch == ' ')
				i ++ ;
		}
		if(i < 2)
		{
			if(i == 1)
			{
				string buf = ss.str();
				if(buf == "return")
				{
					printf("exit cmdline\n");
					break;
				}
			}
			printf("input param error \nPlease input again\n");
			continue;
		}
	}
}

void RunService(const string intro, const TServiceAction StartAction)
{
	string ExeName = AppName();
	string ServiceName = ExtractFilename(ExeName);

	// Processing command line arguments
	int param_count = ParamCount();
	if (param_count > 0)
	{
		if (param_count == 1)
		{
			SigNum = 0;
			string ps = ParamStr(1);
			if (ps.compare("-stop") == 0)
				SigNum = SIGTERM;
			else
				if (ps.compare("-restart") == 0)
					SigNum = SIGHUP;
				else
					if (ps.compare("-install") == 0)
						if (Install(intro))
						{
							printf("%s installed OK.\n", ServiceName.c_str());
							exit(0);
						}
						else
						{
							printf("%s install error : %s\n", ServiceName.c_str(), LastError.c_str());
							exit(1);
						}
					else
					{
						if (ps.compare("-uninstall") == 0)
						{
							if (Uninstall())
							{
								printf("%s uninstalled OK.\n", ServiceName.c_str());
								exit(0);
							}
							else
							{
								printf("%s uninstall error : %s\n", ServiceName.c_str(), LastError.c_str());
								exit(1);
							}
						}
					}
			if (SigNum != 0)
			{
				pid = GetServicePID(ServiceName);
				if (pid != 0)
					if (kill(pid, SigNum) == 0)
					{
						exit(0);
					}
				if (kill(pid, SIGKILL) == 0)
				{
					exit(0);
				}
				printf("%s : cannot do %s. Maybe the daemon is not running?\n", ServiceName.c_str(), ps.c_str());
				{
					exit(0);
				}
			}
		}
		if(param_count == 2)
		{
			string ps = ParamStr(1);
			if(ps == "open")
			{
				ps = ParamStr(2);
				if(ps == "cmdline")
					RunComdLine();
			}else
			{
				printf("Use %s open cmdline.\n", AppName().c_str());
			}
		}
		else
			printf("Use %s -stop to stop the daemon or %s -restart to restart it.\n", ServiceName.c_str(), ServiceName.c_str());
		exit(0);
	}

  // Daemon initialization code

	pid = fork();
	if (pid == -1)            // fork failed
	{
		printf("fork error.\n");
		exit(1);
	}
	else
		if (pid != 0) exit(0); // parent process exits

	pid = setsid();
	if (pid == -1)  // setsid failed
	{
		printf("setsid error.\n");
		exit(1);
	}
#if 1
	#ifndef H_DEBUG
	int fd = open("/dev/null", O_RDWR);
	dup2(fd, 0);
	dup2(fd, 1);
	dup2(fd, 2);
	if (fd > 2) close(fd);
	#endif
#endif
	// Now process runs in the daemon mode
	RegisterService(ServiceName);
	signal(SIGTERM, TranslateSig);
	signal(SIGHUP, TranslateSig);
	signal(SIGPIPE,TranslateSig);
	sigfillset(&SigSet);
	sigdelset(&SigSet, SIGTERM);
	sigdelset(&SigSet, SIGHUP);
	SigNum = 0;
	while (SigNum != SIGTERM)
	{
		pid = fork();
		if (pid == -1) exit(1);
		if (pid != 0) break;  // old process exits
		// Now we are in the new child process
		RegisterService(ServiceName);
		(*StartAction)();
	}
}

bool DirectoryExists(const string DirName)
{
	struct stat filestat;
	if (stat(DirName.c_str(), &filestat) != 0) return false;
	return S_ISDIR(filestat.st_mode);
}

bool Mkdir(const string DirName)
{
	return (mkdir(DirName.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0);
}

bool ForceDirectories(const string DirName)
{
	if (DirectoryExists(DirName)) return true;
	bool f = ForceDirectories(ExtractFilepath(DirName));
	if (!f) return false;
	return Mkdir(DirName);
}

bool RenameFile(const string src, const string dst)
{
  return !rename(src.c_str(), dst.c_str());
}

void StringReplace(string& strBig, const string strsrc, const string strdst)
{
     string::size_type pos = 0;
     while( (pos = strBig.find(strsrc, pos)) != string::npos)
     {
         strBig.replace(pos, strsrc.length(), strdst);
         pos += strdst.length();
     }
}

void Dirs(const string Path, const string SubParam, const bool IncludeSubDir, TStringList2& DirResult, pInt64 TotalSize)
{
	DIR * dir;
	struct dirent * ptr;
	dir = opendir(Path.c_str());
	string pn = Path;
	int l = pn.length();
	if ((l > 0) && (pn.at(l - 1) != '/')) pn += '/';

	while((ptr = readdir(dir)) != NULL)
	{
	  if (ptr->d_type & DT_DIR)
	  {
		if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0)) continue;
		string np = pn + string(ptr->d_name);
		DirResult.push_back(np);
		if (IncludeSubDir) Dirs(np, SubParam, true, DirResult, TotalSize);
	  }
	}
	closedir(dir);
}

void DirFiles(const string Path, const string SubParam, const bool IncludeSubDir, TStringList2& DirResult, pInt64 TotalSize)
{
	DIR * dir;
	struct dirent * ptr;

	string pn = Path;
	int l = pn.length();
	if ((l > 0) && (pn.at(l - 1) != '/')) pn += '/';

	dir = opendir(Path.c_str());
	while((ptr = readdir(dir)) != NULL)
	{
	  if (!(ptr->d_type & DT_DIR))
	  {
		string np = pn + string(ptr->d_name);
		DirResult.push_back(np);
	  }
	}
	closedir(dir);

	dir = opendir(Path.c_str());
	while((ptr = readdir(dir)) != NULL)
	{
	  if (ptr->d_type & DT_DIR)
	  {
		if (!strcmp(ptr->d_name, ".") || !strcmp(ptr->d_name, "..")) continue;
		string np = pn + string(ptr->d_name);
		if (IncludeSubDir) DirFiles(np, SubParam, true, DirResult, TotalSize);
	  }
	}
	closedir(dir);
}

bool DeleteFile(const string src)
{
	return !remove(src.c_str());
}

int LastDelimiter(const string s)
{
	int i = s.length();
	for(int j = i - 1; j > -1; j--)
	{
		if (s.at(j) == '.') return j;
		if (s.at(j) == '/') return -1;
	}
	return -1;
}

string ChangeFileExt(const string Filename, const string ext)
{
	string s;
	int i = LastDelimiter(Filename);
	if (i == -1)
		s = Filename + ext;
	else if (i == 0)
		s = ext;
	else
	{
        char *buf = new char[i + 1];
        memcpy(buf, Filename.c_str(), i);
        buf[i] = 0;
        s = string(buf) + ext;
        free(buf);
	}
	return s;
}

string ExtractFileExt(const string Filename)
{
	string s;
	int i = LastDelimiter(Filename);
	if (i < 1)
		s = "";
	else
	{
        char *buf = strdup(Filename.c_str() + i);
        s = string(buf);
        free(buf);
	}
	return s;
}

bool TryStrToWord(const string s, pWord w)
{
    int i = s.length();
    int k = 0;
    *w = 0;
    for (int j = 0; j < i; j++)
    {
    	char c = s.at(j);
    	if ((c >= '0') && (c <= '9'))
    	{
    		*w = *w * 10 + (c - '0');
    		k++;
    	}
    }
    return k;
}

int CompareMem(const void* src, const void* dst, const int len)
{
	return memcmp(src, dst, len);
}

int StrToInt(const string s)
{
	return atoi(s.c_str());
}

uint StrToUInt(const string s)
{
	return (uint)atoll(s.c_str());
}

Int64 StrToInt64(const string s)
{
	return atoll(s.c_str());
}

UInt64 StrToUInt64(const string s)
{
	return atoll(s.c_str());
}

double StrToFloat(const string s)
{
	return atof(s.c_str());
}

string MidStr(const string src, const int from, const int len)
{
	return src.substr(from - 1, len);
}

string LeftStr(const string src, const int len)
{
	return src.substr(0, len);
}

string RightStr(const string src, const int len)
{
	return src.substr(src.length() - len, len);
}

void Move(const void* src, void* dst, const int len)
{
	memcpy(dst, src, len);
}

void Inc(int& dst, int src)
{
	dst += src;
}

void Inc(Int64& dst, int src)
{
	dst += src;
}

void Inc(Int64& dst, Int64 src)
{
	dst += src;
}

void Inc(int& dst)
{
	dst++;
}

void Inc(Int64& dst)
{
	dst++;
}

void Dec(int& dst, int src)
{
	dst -= src;
}

void Dec(int& dst)
{
	dst--;
}

string Trim(const string s)
{
	stringstream buf;
	stringstream buf2;
	string cache;
	bool data = false;
	bool chunck = false;
	for(int i = 0; i < (int)s.size(); i++)
	{
		char c = s[i];
		switch(c)
		{
			case 32:
			case 9:
			case 13:
			case 10:
			{
				if (!data) continue;
				if (!chunck) chunck = true;
				buf2.write(&c, 1);
				break;
			}
			default:
			{
				if (!data) data = true;
				if (chunck)
				{
					cache = buf2.str();
					buf << cache;
					buf2.str("");
					chunck = false;
				}
				buf.write(&c, 1);
			}
		}
	}
	cache = buf.str();
	buf2.str("");
	buf.str("");
	return cache;
}

bool TryStrToInt(const string s, int& i)
{
	i = StrToInt(s);
	return true;
}

string BoolToStr(bool b, bool alpha)
{
	if (alpha) return (b)?"true":"false"; else return (b)?"1":"0";
}

int PosChar(const Byte c, const pByte src, const int len)
{
	pByte t = src;
	for(int i = 0; i < len; i++)
	{
		if ((*t) == c) return ++i; else ++t;
	}
	return 0;
}

int PosWord(const Word c, const pWord src, const int len)
{
	pWord t = src;
	for(int i = 1; i < len; i++)
	{
		if ((*t) == c) return i; else t = pWord(pByte(t) + 1);
	}
	return 0;
}

int PosCardinal(const Cardinal c, const pCardinal src, const int len)
{
	pCardinal t = src;
	for(int i = 1; i < (len - 2); i++)
	{
		if ((*t) == c) return i; else t = pCardinal(pByte(t) + 1);
	}
	return 0;
}

int Length(const string s)
{
	return s.length();
}

void UpperCase_(string& s)
{
	transform(s.begin(), s.end(), s.begin(),  (int(*)(int))toupper);
}

string UpperCase(const string s)
{
	string t = s;
	UpperCase_(t);
	return t;
}

void LowerCase_(string& s)
{
	transform(s.begin(), s.end(), s.begin(),  (int(*)(int))tolower);
}

string LowerCase(const string s)
{
	string t = s;
	LowerCase_(t);
	return t;
}

int Pos(const string substr, const string src)
{
	return src.find(substr, 0) + 1;
}

Byte toHex(const Byte &x)
{
    return x > 9 ? x -10 + 'A': x + '0';
}

Byte fromHex(const Byte &x)
{
    return isdigit(x) ? x-'0' : x-'A'+10;
}

string URLEncode(const string sIn)
{
    string sOut;
    for( size_t ix = 0; ix < sIn.size(); ix++ )
    {
        Byte buf[4];
        memset( buf, 0, 4 );
        if( isalnum( (Byte)sIn[ix] ) )
        {
            buf[0] = sIn[ix];
        }
        //else if ( isspace( (BYTE)sIn[ix] ) ) //貌似把空格编码成%20或者+都可以
        //{
        //    buf[0] = '+';
        //}
        else
        {
            buf[0] = '%';
            buf[1] = toHex( (Byte)sIn[ix] >> 4 );
            buf[2] = toHex( (Byte)sIn[ix] % 16);
        }
        sOut += (char *)buf;
    }
    return sOut;
};

string URLDecode(const string sIn)
{
    string sOut;
    for( size_t ix = 0; ix < sIn.size(); ix++ )
    {
        Byte ch = 0;
        if(sIn[ix]=='%')
        {
            ch = (fromHex(sIn[ix+1])<<4);
            ch |= fromHex(sIn[ix+2]);
            ix += 2;
        }
        else if(sIn[ix] == '+')
        {
            ch = ' ';
        }
        else
        {
            ch = sIn[ix];
        }
        sOut += (char)ch;
    }
    return sOut;
}

string RandomString(const int len)
{
    string b = "";
    b.resize(len);
	for (int i = 0; i < len; i++) b[i] = charmap[random() % 64];
	return b;
}

string Seal(const string s)
{
  int j = Length(s);
  string s1 = s;
  string ret = s;
  Byte k = 0;
  for (int i = j - 1; i > -1; i--)
  {
    ret[i] = (255 - Byte(s1[i]) + k) % 256;
    k = Byte(s1[i]);
  }
  return ret;
}

string Unseal(const string s)
{
  int j = Length(s);
  string s1 = s;
  string ret = s;
  Byte k = 0;
  for (int i = j - 1; i > -1; i--)
  {
	ret[i] = Byte(255 - ((256 + Byte(s1[i]) - k) % 256));
    k = ret[i];
  }
  return ret;
}

string SealToString(const string s)
{
  int j = Length(s);
  string s1;
  string s2 = s;
  s1.resize(j << 1, ' ');
  for(int i = 0; i < j; i++)
  {
    s1[i << 1] = toHex(Byte(s2[i]) / 16);
    s1[(i << 1) + 1] = toHex(Byte(s2[i]) % 16);
  }
  return UpperCase(s1);
}

string StringToSeal(const string s)
{
  int j = Length(s);
  string s1 = UpperCase(s);
  string s2;
  s2.resize(j >> 1, ' ');
  j >>= 1;
  for(int i = 0; i < j; i++)
  {
	s2[i] = (fromHex(s1[i << 1]) << 4) + fromHex(s1[(i << 1) + 1]);
  }
  return s2;
}

string FormatByte(Int64 i)
{
	double f = i;
	return FormatByte(f);
}

string FormatByte(int i)
{
	double f = i;
	return FormatByte(f);
}

string FormatByte(double f)
{
	char* p = (char*)malloc(32);
	memset(p, 0, 32);
	if (f < 1024)
	    sprintf(p, "%d", int(f + 0.1));
	else if (f < 1048576)
		sprintf(p, "%.2f K", f / 1024);
	else if (f < 1073741824)
		sprintf(p, "%.2f M", f / 1048576);
	else if (f < 1099511627776.0f)
		sprintf(p, "%.2f G", f / 1073741824);
	else if (f < 1125899906842624.0f)
		sprintf(p, "%.2f T", f / 1099511627776.0f);
	else
		sprintf(p, "%.2f P", f / 1125899906842624.0f);
	string s(p);
	free(p);
	return s;
}

string DecodePassword(const string password)
{
  try
  {
   string s = Unseal(StringToSeal(password));
   int i = Length(s);
   if (i > 4)
    return MidStr(s, 3, i - 4);
   else
    return RandomString(20);
  }
  catch(...)
  {
	return RandomString(20);
  }
}

string EncodePassword(const string password)
{
  return SealToString(Seal(RandomString(2) + password + RandomString(2)));
}

string DecodeStringBase64(const string src)
{
	char *dst = base64_decode(src.c_str(), src.size());
	string s(dst);
	free(dst);
	return s;
}

string EncodeStringBase64(const string src)
{
	char *dst = base64_encode(src.c_str(), src.size());
	string s(dst);
	free(dst);
	return s;
}

#ifdef H_ICONV

int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
	iconv_t cd;
	char **pin = &inbuf;
	char **pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);
	if (cd==0) return -1;
	memset(outbuf,0,outlen);
	if ((int)iconv(cd,pin,(size_t*)(&inlen),pout,(size_t*)(&outlen))==-1) return -1;
	iconv_close(cd);
	return 0;
}

char H_CP65001[] = {"utf-8"};
char H_CP936[] = {"gb2312"};

string UTF8ToCP936(string s)
{
	int i = s.length();
	if (!i) return "";
	int j = i << 1;
	char* p = (char*)malloc(j);
	char* b = (char*)malloc(i);
	Move(s.c_str(), b, i);
	code_convert(H_CP65001, H_CP936, b, s.size(), p, j);
	free(b);
	string ss(p);
	free(p);
	return ss;
}

string CP936ToUTF8(string s)
{
	int i = s.length();
	if (!i) return "";
	int j = i << 1;
	char* p = (char*)malloc(j);
	char* b = (char*)malloc(i);
	Move(s.c_str(), b, i);
	code_convert(H_CP936, H_CP65001, b, s.size(), p, j);
	string ss(p);
	free(p);free(b);
	return ss;
}

#endif

string FloatToStr(const double d, string format)
{
	char* p = (char*)malloc(32);
	memset(p, 0, 32);
	sprintf(p, format.c_str(), d);
	string ret(p);
	free(p);
	return ret;
}

string md5(const string src)
{
	char* buf = (char*)malloc(33);
	memset(buf, 0, 33);
	strmd5(src.c_str(), src.size(), buf);
	string ret(buf);
	free(buf);
	return ret;
}

#endif /* PASUTILS_C_ */





