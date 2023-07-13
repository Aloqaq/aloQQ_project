#include "widget.h"
#include "ui_widget.h"
#include <QDataStream>
#include <QMessageBox>
#include <QDateTimeEdit>
#include <QColorDialog>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
Widget::Widget(QWidget *parent,QString name)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    //初始化操作
    udpSocket=new QUdpSocket(this);
    //用户名获取
    uName=name;
    //端口号
    this->port=9999;
    //绑定端口号
    udpSocket->bind(port,QAbstractSocket::ShareAddress|QAbstractSocket::ReuseAddressHint);//共享地址且断线重连

    //发送新用户进入
    sndMsg(UsrEnter);

    //点击发送按钮发送消息
    connect(ui->sendBtn,&QPushButton::clicked,[=](){
        sndMsg(Msg);
    });

    //监听信号
    connect(udpSocket,&QUdpSocket::readyRead,this,&Widget::ReceiveMessage);

    //点击退出关闭窗口
    connect(ui->exitBtn,&QPushButton::clicked,[=]{
        this->close();
    });



    ///////////////////////辅助功能///////////////////////////////////
    //字体
    connect(ui->fontCbx,&QFontComboBox::currentFontChanged,[=](const QFont &font){//点击的字体设置为当前字体
        ui->msgTxtEdit->setCurrentFont(font);
        ui->msgTxtEdit->setFocus();
    });


    //字号
    void(QComboBox::*cbxsignal)(const QString &tex)=&QComboBox::currentTextChanged;//由于QCombox下该函数返回值不同，故需要重载
    connect(ui->sizeCbx,cbxsignal,[=](const QString &text){
        ui->msgTxtEdit->setFontPointSize(text.toDouble());
        ui->msgTxtEdit->setFocus();
    });


    //加粗
    connect(ui->boldTBtn,&QToolButton::clicked,[=](bool isCheck){
        if(isCheck){
            ui->msgTxtEdit->setFontWeight(QFont::Bold);
        }else
        {
            ui->msgTxtEdit->setFontWeight(QFont::Normal);
        }

    });

    //倾斜
    connect(ui->italicTBtn,&QToolButton::clicked,[=](bool check){
        ui->msgTxtEdit->setFontItalic(check);
    });

    //下划线
    connect(ui->underlineTBtn,&QToolButton::clicked,[=](bool check){
        ui->msgTxtEdit->setFontUnderline(check);
    });

    //字体颜色——未设置该功能
//    connect(ui->colorTBtn,&QToolButton::clicked,[=](){
//        QColor color =QColorDialog::getColor(Qt::red);
//        ui->msgTxtEdit->setTextColor(color);
//    })

    //清空聊天记录
    connect(ui->clearTBtn,&QToolButton::clicked,[=](){
        ui->msgBrowser->clear();
    });


    //保存聊天记录
    connect(ui->saveTBtn,&QToolButton::clicked,[=](){
        if(ui->msgBrowser->document()->isEmpty()){
            QMessageBox::warning(this,"警告","内容不能为空");
            return;
        }else{

            QString path=QFileDialog::getSaveFileName(this,"保存记录","聊天记录","(*.txt)");

            if(path.isEmpty()){
                QMessageBox::warning(this,"警告","路径不能为空");
                return;
            }
            else
            {
                QFile file(path);

                //打开模式加换行操作
                file.open(QIODevice::WriteOnly|QIODevice::Text);
                QTextStream stream(&file);
                stream<<ui->msgBrowser->toPlainText();
                file.close();
            }

        }


    });
}


//接受UDP消息
void Widget::ReceiveMessage(){
    //获取数据
    //获取长度
    qint64 size=udpSocket->pendingDatagramSize();
    QByteArray array=QByteArray(10000,0);
    udpSocket->readDatagram(array.data(),size);

    //解析数据
    //第一段 类型     //第二段 用户名  //第三段 内容
    QDataStream stream(&array,QIODevice::ReadOnly);
    int msgType;//读取到类型
    QString usrName;
    QString msg;


    //获取当前时间
    QString time=QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    stream>>msgType;

    switch (msgType) {
    case Msg:
        stream>>usrName>>msg;

        //追加聊天记录
        ui->msgBrowser->setTextColor(Qt::blue);
        ui->msgBrowser->append("["+usrName+"]"+time);
        ui->msgBrowser->append(msg);

        break;
    case UsrEnter:
        //更新右侧TableWidget
        stream>>usrName;
        usrEnter(usrName);
        break;

    case UsrLeft:
        stream>>usrName;
        userLeft(usrName,time);//当前时间已经过获取
        break;


    default:
        break;
    }
}


//处理新用户的加入
void Widget::usrEnter(QString usrName)
{
    bool flag=ui->usrTblWidget->findItems(usrName,Qt::MatchExactly).isEmpty();
    if(flag){
        QTableWidgetItem *usr=new QTableWidgetItem(usrName);

        //插入行
        ui->usrTblWidget->insertRow(0);
        ui->usrTblWidget->setItem(0,0,usr);

        //追加聊天记录
        ui->msgBrowser->setTextColor(Qt::gray);
        ui->msgBrowser->append(QString("%1 上线了").arg(usrName));

        //在线人数更新
        ui->usrNumLbl->setText(QString("在线用户：%1人").arg(ui->usrTblWidget->rowCount()));

        //把自己信息广播出去,保证上线信息同步
        //不会陷入死循环，只有信息empty才会执行
        sndMsg(UsrEnter);
    }
}

//处理用户离开
void Widget::userLeft(QString usrName,QString time){

    //更新右侧TableWidget
    bool flag=ui->usrTblWidget->findItems(usrName,Qt::MatchExactly).isEmpty();
    if(!flag){
        int row=ui->usrTblWidget->findItems(usrName,Qt::MatchExactly).first()->row();
        ui->usrTblWidget->removeRow(row);

        //追加聊天记录
        ui->msgBrowser->setTextColor(Qt::gray);
        ui->msgBrowser->append(QString("%1 于 %2 离开").arg(usrName).arg(time));

        //更新在线人数
        ui->usrNumLbl->setText(QString("在线用户：%1人").arg(ui->usrTblWidget->rowCount()));
    }

}




//获取用户姓名
QString Widget::getUsr(){
    return this->uName;
}


//广播udp消息
void Widget::sndMsg(MsgType type)
{
    //消息分为三种类型
    //发送的数据 做分段处理 第一段 类型 第二段具体内容
    QByteArray array;

    QDataStream stream(&array,QIODevice::WriteOnly);

    //第一段内容添加到流
    stream<<type<<getUsr();

    switch (type) {
    case Msg://普通消息发送
        if(ui->msgTxtEdit->toPlainText()==""){
            QMessageBox::warning(this,"警告","发送内容不能为空");
            return;
        }

        //第二段数据 具体说的话
        stream<<getMsg();
        break;
    case UsrEnter:
        break;
    case UsrLeft:
        break;
    default:
        break;
    }

    //书写报文 广播发送255.255.255.255
    udpSocket->writeDatagram(array,QHostAddress::Broadcast,port);


}


QString Widget::getMsg()//获取聊天信息
{
    QString str=ui->msgTxtEdit->toHtml();//html保存格式

    //发送完成清空内容框
    ui->msgTxtEdit->clear();
    ui->msgTxtEdit->setFocus();//光标返回重置

    return str;
}
//关闭事件
void Widget::closeEvent(QCloseEvent * e){
    emit this->closeWidget();
    sndMsg(UsrLeft);//发送用户离开信息

    //断开套接字——关闭再销毁
    udpSocket->close();
    udpSocket->destroyed();

    QWidget::closeEvent(e);
}


Widget::~Widget()
{
    delete ui;
}

