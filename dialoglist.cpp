#include "dialoglist.h"
#include "ui_dialoglist.h"
#include <QToolButton>
#include "widget.h"
#include <QMessageBox>
DialogList::DialogList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DialogList)
{
    ui->setupUi(this);

    //设置标题
    setWindowTitle("AloQQ 2023");

    //设置图表
    setWindowIcon(QPixmap(":/images/qq.png"));

    QList<QString>nameList;
    nameList << "水票奇缘" << "忆梦如澜" <<"北京出版人"<<"Cherry"<<"淡然"
             <<"娇娇girl"<<"落水无痕"<<"青墨暖暖";


    QStringList iconNameList; //图标资源列表
    iconNameList << "spqy"<< "ymrl" <<"qq" <<"Cherry"<< "dr"
                 <<"jj"<<"lswh"<<"qmnn";

    QVector<QToolButton *>vToolBtn;


    for(int i=0;i<8;i++){
       //设置头像
       QToolButton * btn=new QToolButton;
       //设置文字
       btn->setText(nameList[i]);
       //设置头像
       QString str=QString(":/images/%1.png").arg(iconNameList[i]);
       btn->setIcon(QPixmap(str));
       //设置头像大小
       btn->setIconSize(QPixmap(str).size());
       //设置按钮风格
       btn->setAutoRaise(true);
       //设置文字和图一起显示
       btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
       //加载到垂直布局中
       ui->vLayout->addWidget(btn);


       //容器保存住所有按钮
       vToolBtn.push_back(btn);
       //9个标识的默认初始化
       isShow.push_back(false);
    }

    //对按钮添加信号槽
    for(int i=0;i<vToolBtn.size();i++){
        connect(vToolBtn[i],&QToolButton::clicked,[=](){

            //如果打开了就不要再次打开
            if(isShow[i]){
                QString str=QString("%1的窗口已被打开").arg(vToolBtn[i]->text());
                QMessageBox::warning(this,"警告",str);
                return ;
            }
            isShow[i]=true;
            //弹出聊天对话框
            //构造聊天窗口时，告诉窗口他的名字 参数1 顶层方式弹出 参数2 窗口名字
            //注意：初始的Widget构造函数并没有这两个参数
            Widget *widget=new Widget(0,vToolBtn[i]->text());//0表示以顶层当时弹出，清除聊天记录不会被删掉


            //设置窗口标题
            widget->setWindowTitle(vToolBtn[i]->text());
            widget->setWindowIcon(vToolBtn[i]->icon());
            widget->show();


            //启动关闭连接
            connect(widget,&Widget::closeWidget,[=](){
                isShow[i]=false;
            });
        });
    }

}

DialogList::~DialogList()
{
    delete ui;
}
