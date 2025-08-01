#include <iostream>
#include <fstream>

#ifndef HELPER_NOPLUGINS
#include "Plugins.h"
#endif

#include "Storage.h"
#include "AliasManager.h"

#include "ConverterApp.h"


static constexpr const char* STORAGE_DIR = "./storage/";

int main(int argc, char* argv[])
{
	auto aliasManager = new AliasManager("aliasmanager", STORAGE_DIR);
	aliasManager->init();

	assert(argc > 1);

	auto appName = std::string(argv[1]);


	tolowercase(appName);

	////std::vector<std::string> args;
	////for (size_t i = 2; i < argc; ++i)
	////	args.push_back({ argv[i] });

	//if (appName == "converter") {
	//	auto a = ConverterApp();
	//	auto st = a.run(argv + 2, argc - 2);

	//	if (st.isError()) std::cout << st.msg << std::endl;

	//	assert(st.isOk());
	//}


	return 0;
}


//class Example {
//
//public:
//	int val1;
//	std::string nm;
//
//};
//
//Status conv(const Example* e, Storage::RecordBuffer *s) {
//	std::cout << "converting..." << std::endl;
//	s->writeIntField("val1", e->val1);
//	s->writeStringField("val::nm", e->nm);
//	return Status(StatusCode::SC_OK);
//}
//Status deconv(Example** e, const Storage::RecordView* v) {
//	std::cout << "converting..." << std::endl;
//	
//	*e = new Example();
//	v->readIntField("val1", &((*e)->val1));
//	v->readStringField("val::nm", &((*e)->nm));
//	
//	return Status(StatusCode::SC_OK);
//}
//
//
//struct Example2 {
//	const char* msg;
//	Example* e;
//};
//
//Status ser(const Example2* e, Storage::RecordBuffer* s) {
//	std::cout << "ser ex2" << std::endl;
//
//	s->writeStringField("msg", e->msg);
//	s->writeRecordField("example", e->e);
//
//	return Status::OK();
//}
//Status des(Example2** e, const Storage::RecordView* v) {
//	std::cout << "des ex2" << std::endl;
//
//	*e = new Example2{};
//	v->readCStringField("msg", &((*e)->msg));
//	v->readRecordField("example", &((*e)->e));
//
//	return Status::OK();
//}
//
//
//int main(int argc, char* argv[])
//{
//
//#if 0
//	{
//		auto st = new Storage("testrec");
//		st->registerTypeSerializer<Example>(conv);
//		st->registerTypeDeserializer<Example>(deconv);
//
//		Example e{
//			213,
//			"hello"
//		};
//		auto s1 = st->writeRecord("e-record", &e);
//
//		Example* read = NULL;
//		auto s2 = st->readRecord("e-record", &read);
//
//		// --
//		auto s3 = st->save();
//	}
//
//	{
//		auto st = new Storage("testrec");
//		//st->registerTypeSerializer<Example>(conv);
//		st->registerTypeDeserializer<Example>(deconv);
//
//		auto s1 = st->load();
//		assert(s1.isOk());
//
//		Example* read = NULL;
//		auto s2 = st->readRecord("e-record", &read);
//	}
//#endif
//
//#if 0
//	{
//		auto st = new Storage("testrec");
//		st->registerTypeSerializer<Example>(conv);
//		st->registerTypeDeserializer<Example>(deconv);
//
//		st->registerTypeSerializer<Example2>(ser);
//		st->registerTypeDeserializer<Example2>(des);
//
//
//		Example e{
//			213,
//			"hello"
//		};
//		Example2 e2{
//			"world",
//			&e,
//		};
//
//		auto s1 = st->writeRecord("recurrec", &e2);
//
//		Example2* read = NULL;
//		auto s2 = st->readRecord("recurrec", &read);
//
//		// --
//		auto s3 = st->save();
//	}
//
//	{
//		auto st = new Storage("testrec");
//		st->registerTypeDeserializer<Example>(deconv);
//		st->registerTypeDeserializer<Example2>(des);
//
//
//		DEBUG_LOG_DIVIDERB();
//		DEBUG_LOG_LINE();
//		auto s1 = st->load();
//		assert(s1.isOk());
//
//		Example2* read = NULL;
//		auto s2 = st->readRecord("recurrec", &read);
//	}
//#endif
//
//
//
//#if 1
//	{
//		auto st = new Storage("storage");
//		st->registerTypeSerializer<Example>(conv);
//		st->registerTypeDeserializer<Example>(deconv);
//
//		st->registerTypeSerializer<Example2>(ser);
//		st->registerTypeDeserializer<Example2>(des);
//
//		Example e{
//			213,
//			"hello"
//		};
//		Example2 e2{
//			"world",
//			&e,
//		};
//
//		auto s1 = st->writeRecord("recurrec", &e2);
//		assert(s1.isOk());
//
//		auto ss1 = st->writeKVString("hello", "world");
//		auto ss2 = st->writeKVString("wrld", "hll");
//		auto ss3 = st->writeKVInt("Number of Usages", -553);
//		auto ss4 = st->writeKVInt("Number of Usages (abs)", 553);
//
//		assert(ss1.isOk() && ss2.isOk() && ss3.isOk() && ss4.isOk());
//
//		Example2* read = NULL;
//		auto s2 = st->readRecord("recurrec", &read);
//		assert(s2.isOk());
//
//		// --
//		auto s3 = st->save();
//		assert(s3.isOk());
//	}
//
//	{
//
//		auto st = new Storage("storage");
//		st->registerTypeDeserializer<Example>(deconv);
//		st->registerTypeDeserializer<Example2>(des);
//
//
//		DEBUG_LOG_DIVIDERB();
//		DEBUG_LOG_LINE();
//		auto s1 = st->load();
//		assert(s1.isOk());
//
//		Example2* read = NULL;
//		auto s2 = st->readRecord("recurrec", &read);
//		assert(s2.isOk());
//
//
//		const char* v1;
//		const char* v2;
//		int v3;
//		int v4;
//
//		auto ss1 = st->readKVString("hello", &v1);
//		auto ss2 = st->readKVString("wrld", &v2);
//		auto ss3 = st->readKVInt("Number of Usages", &v3);
//		auto ss4 = st->readKVInt("Number of Usages (abs)", &v4);
//
//		assert(ss1.isOk() && ss2.isOk() && ss3.isOk() && ss4.isOk());
//	}
//
//#endif
//
//}

//int main(int argc, char* argv[])
//{
//	//Example e{};
//	//const type_info& ti = typeid(e);
//	//auto a = typeid(Example()).hash_code();
//	//std::cout << typeid(Example()).name() << std::endl;
//
//	if(1)
//	{
//		auto st = new Storage("test");
//
//		auto _st = st->writeKVInt("usages", 12);
//		assert(_st.isOk());
//		auto __st = st->writeKVString("name", "vscode");
//		assert(__st.isOk());
//
//		int a = 1;
//
//		auto s = st->save();
//
//		assert(s.isOk());
//		auto ss = s;
//	}
//
//
//	{
//		auto str = new Storage("test");
//		auto _s = str->load();
//		assert(_s.isOk());
//		int a;
//	}
//
//
//}

//int main(int argc, char* argv[])
//{
//	std::cout << "loading plugin" << std::endl;
//	
//	//todo argc > 1;
//	const char* pluginName = argv[1];
//	const char** plargs = new const char* [argc - 2];
//
//	for (int i = 2; i < argc; i++) {
//		//std::cout << i << ": '" << argv[i] << "'" << std::endl;
//		plargs[i - 2] = argv[i];
//	}
//
//	runPlugin(pluginName, plargs, argc - 2);
//}

//
//int main()
//{
//    std::cout << "Hello World!\n";
//
//    auto res = readPluginFile({ "PluginName", 0 }, ".\\");
//    DEBUG_PRINT_DIVIDER();
//
//
//    auto vstate = validateLoadedPlugin({ "PluginName", 0 }, res);
//
//    auto executableState = extractPluginContent({ "PluginName", 0 }, vstate);
//
//
//    long long plres;
//    char* plmsg;
//    bool r = executePlugin({ "PluginName", 0 }, executableState, new const char* [3] {}, 3, &plres, &plmsg);
//
//    if (!r) {
//        std::cout << "client: false status" << std::endl;
//    }
//    else {
//        std::cout << "client: res: " << plres << ", msg: '" << plmsg << "'" << std::endl;
//        delete[] plmsg;
//    }
//
//}
