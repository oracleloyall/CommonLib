#include <iostream>

#include "pugixml.h"
#define int32_t int
#include<stdio.h>
#include<string>
using namespace std;
static const int32_t nBufSize = 128;
static char szBuf[nBufSize] = { 0 };

void Write(const char *szXmlFileName)
{
    pugi::xml_document xmlDoc;
    pugi::xml_node nodeRoot = xmlDoc.append_child("root");

    pugi::xml_node pre = xmlDoc.prepend_child(pugi::node_declaration);
    pre.append_attribute("version") = "1.0";
    pre.append_attribute("encoding") ="gb2312";



    pugi::xml_node nodeCommentStudents = nodeRoot.append_child(
			pugi::node_comment);
    nodeCommentStudents.set_value("all students info");

    pugi::xml_node nodeStudents = nodeRoot.append_child("students");
    for(int32_t i = 0; i < 10; ++i)
    {
        snprintf(szBuf, nBufSize, "student_%02d", i);
        pugi::xml_node nodeStudent = nodeStudents.append_child("student");

        nodeStudent.append_attribute("name").set_value(szBuf);
        nodeStudent.append_attribute("score").set_value(100 - i);
    }


    pugi::xml_node nodeCommentBooks = nodeRoot.append_child(pugi::node_comment);
    nodeCommentBooks.set_value("all books info");

    pugi::xml_node nodeBooks = nodeRoot.append_child("books");
    for(int32_t i = 0; i < 10; ++i)
    {
        snprintf(szBuf, nBufSize, "book_%02d", i);
        pugi::xml_node nodeBook = nodeBooks.append_child("book");

        nodeBook.append_attribute("book").set_value(szBuf);
        nodeBook.append_attribute("price").set_value(50 - i);    }

    xmlDoc.save_file(szXmlFileName, "\t", 1U, pugi::encoding_utf8);
}


void Read(const char *szXmlFileName)
{
    pugi::xml_document xmlDoc;
    if(!xmlDoc.load_file(szXmlFileName, pugi::parse_default, pugi::encoding_utf8))
    {
        std::cout << "read " << szXmlFileName << " failed" << std::endl;
        return;
    }
    xmlDoc.load_file(szXmlFileName, pugi::parse_default, pugi::encoding_utf8);

    pugi::xml_node nodeRoot = xmlDoc.child("root");

    for(pugi::xml_node node = nodeRoot.child("students").first_child(); node; node = node.next_sibling())
    {
        std::cout << "\t" << node.attribute("name").value() << "," << node.attribute("score").value() << std::endl;
    }
    std::cout << std::endl;

    for(pugi::xml_node node = nodeRoot.child("books").first_child(); node; node = node.next_sibling())
    {
        std::cout << "\t" << node.attribute("book").value() << "," << node.attribute("price").value() << std::endl;
    }
}
int size = 1;
void AlarmWrite(const char *szXmlFileName) {
	pugi::xml_document xmlDoc;
//fmcontent 报文体
	pugi::xml_node nodeRoot = xmlDoc.append_child("fmcontent");

	pugi::xml_node pre = xmlDoc.prepend_child(pugi::node_declaration);
	pre.append_attribute("version") = "1.0";
	pre.append_attribute("encoding") = "gb2312";
//alarmcount 本次送达告警数量
	pugi::xml_node nodeAlarmCount = nodeRoot.append_child("alarmcount");
	nodeAlarmCount.append_child(pugi::node_pcdata).set_value("3");

	pugi::xml_node nodeAlarms = nodeRoot.append_child("alarms");
	for (int i = 1; i <= size; i++) {
		pugi::xml_node nodeAlarmsChild = nodeAlarms.append_child("alarm");
//alarmseq 告警序号
		pugi::xml_node alarmseq = nodeAlarmsChild.append_child("alarmseq");
		string s;
		sprintf(const_cast<char*>(s.c_str()), "%d", i);
		alarmseq.append_child(pugi::node_pcdata).set_value(s.c_str());
//fmkpiid 告警KPI编号
		pugi::xml_node fmkpiid = nodeAlarmsChild.append_child("fmkpiid");
		fmkpiid.append_child(pugi::node_pcdata).set_value("FT_01_01_0001_0005");
//fmtitle 告警标题
		pugi::xml_node fmtitle = nodeAlarmsChild.append_child("fmtitle");
		fmtitle.append_child(pugi::node_pcdata).set_value("检查数据库锁死");
//dn 对象唯一标识 value:boss.collection.coll_point:10.223.26.28
		pugi::xml_node dn = nodeAlarmsChild.append_child("dn");
		dn.append_child(pugi::node_pcdata).set_value(
				"boss.collection.coll_point:10.223.26.28");
//severity  告警级别1,2,3
		pugi::xml_node severity = nodeAlarmsChild.append_child("severity");
		severity.append_child(pugi::node_pcdata).set_value("1");
//redefineseverity 告警级别重定义minor， 严重major， 主要 critical 一般
		//如何确定告警级别含义？？？？
		pugi::xml_node redefineseverity = nodeAlarmsChild.append_child(
				"redefineseverity");
		redefineseverity.append_child(pugi::node_pcdata).set_value("major");
//activestatus告警状态NEW:激活告警,ACK:确认告警,CLEAR:清除告警  //alarmFlag
		pugi::xml_node activestatus = nodeAlarmsChild.append_child(
				"activestatus");
		activestatus.append_child(pugi::node_pcdata).set_value("NEW");
//alarmtime 告警发生时时间，
		pugi::xml_node alarmtime = nodeAlarmsChild.append_child("alarmtime");
		alarmtime.append_child(pugi::node_pcdata).set_value(
				"2018-01-18 15:04:01");
//collectbegintime 采集开始时间，
		pugi::xml_node collectbegintime = nodeAlarmsChild.append_child(
				"collectbegintime");
		collectbegintime.append_child(pugi::node_pcdata).set_value(
				"2018-01-18 15:04:01");
//collectendtime 采集结束时间
		pugi::xml_node collectendtime = nodeAlarmsChild.append_child(
				"collectendtime");
		collectendtime.append_child(pugi::node_pcdata).set_value(
				"2018-01-18 15:04:01");
		//最后的节点加入cleartime 清除时间
		if (i == size) {
			pugi::xml_node cleartime = nodeAlarmsChild.append_child(
					"cleartime");
			cleartime.append_child(pugi::node_pcdata).set_value(
					"2018-01-18 15:04:01");
		}
//alarmtxt 告警内容
		pugi::xml_node alarmtxt = nodeAlarmsChild.append_child("alarmtxt");
		alarmtxt.append_child(pugi::node_pcdata).set_value(
				"Web01主机检查数据库锁数据超过阀值35");


	}
	xml_string_writer writer;
	//format_write_bom
	nodeRoot.print(writer, "", 0x02);
	cout << writer.xml << endl;
	//alarms
	xmlDoc.save_file(szXmlFileName, "\t", 1U, pugi::encoding_utf8);
}
void AlarmRead(const char *szXmlFileName) {
	pugi::xml_document xmlDoc;
	if (!xmlDoc.load_file(szXmlFileName, pugi::parse_default,
			pugi::encoding_utf8)) {
		std::cout << "read " << szXmlFileName << " failed" << std::endl;
		return;
	}
	xmlDoc.load_file(szXmlFileName, pugi::parse_default, pugi::encoding_utf8);

	pugi::xml_node nodeRoot = xmlDoc.child("root");

	for (pugi::xml_node node = nodeRoot.child("students").first_child(); node;
			node = node.next_sibling()) {
		std::cout << "\t" << node.attribute("name").value() << ","
				<< node.attribute("score").value() << std::endl;
	}
	std::cout << std::endl;

	for (pugi::xml_node node = nodeRoot.child("books").first_child(); node;
			node = node.next_sibling()) {
		std::cout << "\t" << node.attribute("book").value() << ","
				<< node.attribute("price").value() << std::endl;
	}
}
extern void socket_poll(const char *addr, int port);
void test() {
	pugi::xml_document doc;
	const char source[] = "<mesh name='sphere'><bounds>0 0 1 1</bounds></mesh>";
	size_t size = sizeof(source);
	// You can use load_buffer_inplace to load document from mutable memory block; the block's lifetime must exceed that of document
	char* buffer = new char[size];
	memcpy(buffer, source, size);

	// The block can be allocated by any method; the block is modified during parsing
	pugi::xml_parse_result result = doc.load_buffer_inplace(buffer, size);
	//pugi::xml_node nodeRoot
	// You have to destroy the block yourself after the document is no longer used
	delete[] buffer;
}
void check_xml(const char* source) {
// tag::code[]
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_string(source);
	if (result) {
		std::cout << "XML [" << source
				<< "] parsed without errors, attr value: ["
				<< doc.child("alarmseq").attribute("result").value() << "]\n\n";

		pugi::xml_node root = doc.child("fmcontent");
		//alarms
		pugi::xml_node alarms = root.child("alarms");
		//alarm
		pugi::xml_node childalarm = alarms.child("alarm");

		pugi::xml_node seq_child = childalarm.child("alarmseq");

		cout << "alarmseq:" << seq_child.text() << endl;
		pugi::xml_node res_child = childalarm.child("result");
		cout << "result :" << res_child.text() << endl;


	} else {
		std::cout << "XML [" << source << "] parsed with errors, attr value: ["
				<< doc.child("alarmseq").attribute("result").value() << "]\n";
		std::cout << "Error description: " << result.description() << "\n";
		std::cout << "Error offset: " << result.offset << " (error at [..."
				<< (source + result.offset) << "]\n\n";
	}
//	if (result) {
//		std::cout << "XML [" << source
//				<< "] parsed without errors, attr value: ["
//				<< doc.child("node").attribute("attr").value() << "]\n\n";
//	} else {
//		std::cout << "XML [" << source << "] parsed with errors, attr value: ["
//				<< doc.child("node").attribute("attr").value() << "]\n";
//		std::cout << "Error description: " << result.description() << "\n";
//		std::cout << "Error offset: " << result.offset << " (error at [..."
//				<< (source + result.offset) << "]\n\n";
//	}
// end::code[]
}

int main() {
	AlarmWrite("alarm.xml");
//	check_xml("<node attr='value'><child>text</child></node>");
//	check_xml("<node attr='value'><child>text</chil></node>");
//	check_xml("<node attr='value'><child>text</child>");
//	check_xml("<node attr='value\"><child>text</child></node>");
//	check_xml("<node attr='value'><#tag /></node>");
	check_xml(
			"<?xml version=\"1.0\" encoding=\"gb2312\" ?><fmcontent><requestid>60</requestid><alarmcount>1</alarmcount><error>succeed</error><alarms><alarm><alarmseq>1</alarmseq><result>1</result></alarm></alarms></fmcontent>");
	char buff[1024] = { "\0" };
	sprintf(buff, "%s",
			"<?xml version=\"1.0\" encoding=\"gb2312\" ?><fmcontent><requestid>60</requestid><alarmcount>1</alarmcount><error>succeed</error><alarms><alarm><alarmseq>2</alarmseq><result>0</result></alarm><alarm><alarmseq>2</alarmseq><result>1</result></alarm></alarms></fmcontent>");
	std::cout << buff + 251 << std::endl;
	check_xml(buff);
	return 0;
}
//int32_t main()
//{
//
//	AlarmWrite("alarm.xml");
//	socket_poll("10.21.17.89", 22);
//    return 0;
//}
