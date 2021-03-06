// 操作系统课设.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"

using namespace std;

const int MAXBLOCK = 256;//indextable,bitmap,fat
const int MAXSUBCATALOG = 50;
const int MAXCATALOG = 50;//catalog
const int MAXFILE = 50;//fcb

string Username, Password;
string Usernameset[8] = { "user1","user2", "user3", "user4", "user5", "user6", "user7", "user8" };
string Passwordset[8] = { "psw1","psw2", "psw3", "psw4", "psw5", "psw6", "psw7", "psw8" };

int NowCatalog = 0;

struct fat; struct indextable; struct subcatalog; struct catalog; struct fcb;

struct fat
{
	int NextBlockNumber;
}Fat[MAXBLOCK];

struct indextable
{
	fcb* File;
	int FileType;//1为文件 2为目录
	int FileLength;
	catalog* Catalog;
}IndexTable[MAXBLOCK];

struct subcatalog
{
	string FileorCatalogName;
	indextable* Index;
};

struct catalog
{
	string CatalogName;
	subcatalog* index;
}Catalog[MAXCATALOG];

struct fcb
{
	string Data;
	int BlockNumber;
	int FirstBlock;
}Fcb[MAXFILE];

bool BitMap[MAXBLOCK];

bool Login()
{
	printf("请输入用户名和密码：\n");
	cin >> Username;
	cin >> Password;
	for (int i = 0; i < 8; i++){
		if (Usernameset[i] == Username && Passwordset[i] == Password) {
			printf("成功登陆。\n");
			return true;
		}
	}
	printf("登录失败，请重新登录。\n");
	return false;
}

//查询当前目录下是否存在重名文件或目录
bool checkSameName(string name, catalog src,int type)
{
	for (int i = 0; i < MAXCATALOG; i++) {
		if (src.index[i].FileorCatalogName == name && src.index[i].Index->FileType == type) {
			return true;
		}
	}
	return false;
}

//根据名字找到目录下文件
fcb* findFile(string FileName, catalog src)
{
	fcb* c = NULL;
	for (int i = 0; i < MAXCATALOG; i++) {
		if (src.index[i].FileorCatalogName == FileName && src.index[i].Index->FileType == 1) {
			c = src.index[i].Index->File;
			return c;
		}
	}
	return c;
}

//根据名字找到目录下子目录
catalog* findCatalog(string CatalogName, catalog src)
{
	catalog* c = NULL;
	for (int i = 0; i < MAXCATALOG; i++) {
		if (src.index[i].FileorCatalogName == CatalogName && src.index[i].Index->FileType == 2) {
			c = src.index[i].Index->Catalog;
			return c;
		}
	}
	return c;
}

bool createCatalog(string NewCatalogName,catalog& SrcCatalog)
{
	if (checkSameName(NewCatalogName, SrcCatalog, 2)) {
		cout << "有重复目录"<<endl;
		return false;
	}
	else {
		int t = 0;
		while (t<MAXCATALOG && SrcCatalog.index[t++].FileorCatalogName != "");//在当前目录中找到一个空余的subcatalog
		if (t == MAXCATALOG) return false; t--;
		SrcCatalog.index[t].FileorCatalogName = NewCatalogName;

		int tt = 0;
		while (tt < MAXBLOCK && IndexTable[tt++].FileType != -2);//在节点表中找到一个空余的节点
		if (tt == MAXBLOCK) return false; tt--;
		SrcCatalog.index[t].Index = &IndexTable[tt];//新建目录指向节点表
		IndexTable[tt].FileLength = 1;
		IndexTable[tt].FileType = 2;//目录
		int ttt = 0; while (Catalog[ttt++].CatalogName != ""); ttt--;//找到一个空余的目录表，把新建目录指向该表
		IndexTable[tt].Catalog = &Catalog[ttt];
		Catalog[ttt].CatalogName = NewCatalogName;
	}
}

//给文件分配空闲块
int allocBlock(fcb& File, int FileLength)
{
	int FirstBlock = -1; int PreBlockId = -1;
	for (int i = 0; i < MAXBLOCK; i++) {
		if (!BitMap[i]) {
			BitMap[i] = true;
			FileLength--;
			if (FirstBlock == -1) {//首块
				FirstBlock = i;
				PreBlockId = i;
			}
			else {
				Fat[PreBlockId].NextBlockNumber = i;
				PreBlockId = i;
			}
		}
		if (FileLength == 0) return FirstBlock;
	}
	return -1;
}

//创建文件
bool createFile(string NewFileName, catalog& SrcCatalog, int FileLength)
{
	if (checkSameName(NewFileName, SrcCatalog, 1)) {
		cout << "有重复目录" << endl;
		return false;
	}
	else {
		int t = 0;
		while (t<MAXCATALOG && SrcCatalog.index[t++].FileorCatalogName != "");//在当前目录中找到一个空余的subcatalog
		if (t == MAXCATALOG) return false; t--;
		SrcCatalog.index[t].FileorCatalogName = NewFileName;

		int tt = 0;
		while (tt < MAXBLOCK && IndexTable[tt++].FileType != -2);//在节点表中找到一个空余的节点
		if (tt == MAXBLOCK) return false; tt--;
		SrcCatalog.index[t].Index = &IndexTable[tt];//新建文件指向节点表
		IndexTable[tt].FileLength = FileLength;
		IndexTable[tt].FileType = 1;//文件

		int ttt = 0;
		while (ttt < MAXFILE && Fcb[ttt++].FirstBlock != -2);
		if (ttt == MAXFILE) return false; ttt--;
		IndexTable[tt].File = &Fcb[ttt];//将新建文件节点与FCB连接

		Fcb[ttt].BlockNumber = FileLength;
		int tmp = allocBlock(Fcb[ttt], FileLength);
		if (tmp != -1) {
			Fcb[ttt].FirstBlock = tmp;
			return true;
		}
		else {
			return false;
		}

	}
}

void changeCatalog(int index)
{
	NowCatalog = index;
}
void changeCatalog(string name)
{

}

//初始化
void initAll(int userid)
{
	for (int i = 0; i < MAXBLOCK; i++) {
		Fat[i].NextBlockNumber = -1;
		IndexTable[i].File = NULL;
		IndexTable[i].Catalog = NULL;
		IndexTable[i].FileLength = -2;
		IndexTable[i].FileType = -2;
	}
	for (int i = 0; i < MAXCATALOG; i++) {
		Catalog[i].index = new subcatalog[MAXSUBCATALOG];
		Catalog[i].CatalogName = "";
		for (int j = 0; j < MAXSUBCATALOG; j++) {
			Catalog[i].index[j].FileorCatalogName = "";
			Catalog[i].index[j].Index = NULL;
		}
	}
	for (int i = 0; i < MAXFILE; i++) {
		Fcb[i].Data = "";
		Fcb[i].BlockNumber = -2;
		Fcb[i].FirstBlock = -2;
	}
	for (int i = 0; i < MAXBLOCK; i++) {
		BitMap[i] = false;
	}

	Catalog[0].CatalogName = "root";
	if (!createCatalog("user1", Catalog[0])) cout << "error";
	if (!createCatalog("user2", Catalog[0])) cout << "error";
	if (!createCatalog("user3", Catalog[0])) cout << "error";
	if (!createCatalog("user4", Catalog[0])) cout << "error";
	if (!createCatalog("user5", Catalog[0])) cout << "error";
	if (!createCatalog("user6", Catalog[0])) cout << "error";
	if (!createCatalog("user7", Catalog[0])) cout << "error";
	if (!createCatalog("user8", Catalog[0])) cout << "error";

	//切换到当前用户目录下
	changeCatalog(userid);
}

void showCatalog() {
	cout << "Catalog:" << endl;
	for (int i = 0; i < MAXCATALOG; i++) {
		if(Catalog[i].CatalogName != "")
			cout << Catalog[i].CatalogName << endl;
		for (int j = 0; j < MAXSUBCATALOG; j++) {
			if (Catalog[i].index[j].FileorCatalogName != "") {
				cout << "   " << Catalog[i].index[j].FileorCatalogName << endl;
			}
			
		}
		cout << endl;
	}
}

void showFat() {
	cout << "FAT:" << endl;
	for (int i = 0; i < MAXBLOCK; i++) {
		if(Fat[i].NextBlockNumber != -1)
			cout << i << " " << Fat[i].NextBlockNumber << endl;
	}
}

void showIndex() {
	cout << "IndexTable:" << endl;
	for (int i = 0; i < MAXBLOCK; i++) {
		if (IndexTable[i].FileType != -2)
			cout << IndexTable[i].FileType << " " << IndexTable[i].FileLength << endl;
	}
}

void showFcb() {
	cout << "FCB:" << endl;
	for (int i = 0; i < MAXFILE; i++) {
		if (Fcb[i].BlockNumber != -2)
			cout << Fcb[i].BlockNumber << " " << Fcb[i].Data << " " << Fcb[i].FirstBlock << endl;
	}
}

void showBitmap() {
	cout << "Bitmap:" << endl; int j = 0;
	for (int i = 0; i < 100; i++) {
		if (BitMap[i]) cout << "1";
		else cout << "0";
		j++;
		if (j == 16) {
			cout << endl;
			j = 0;
		}
	}
}

void showAll()
{
	showCatalog(); showFat(); showIndex(); showFcb(); showBitmap();
}

int main()
{
	//while (!Login());
	/* todo 检查现有文件，读取到内存中*/
	int userid;
	for (userid = 0; Usernameset[userid] != Username; userid++);//找到当前用户
	initAll(userid);
	/*
	catalog* a = findCatalog("user11", Catalog[0]);
	if(a != NULL)cout << a->CatalogName;
	else cout << "error";
	*/
	

	system("pause");
    return 0;
}

