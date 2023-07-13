#ifndef WIDGET_H
#define WIDGET_H
#include <QUdpSocket>

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

    enum MsgType{Msg,UsrEnter,UsrLeft};

public:
    Widget(QWidget *parent,QString name);
    ~Widget();

private:
    Ui::Widget *ui;
signals:
    //关闭窗口发送关闭信息
    void closeWidget();
public:
    //关闭事件
    void closeEvent(QCloseEvent *);

public:
    void sndMsg(MsgType type);//广播udp消息
    void usrEnter(QString usrName);//处理新用户的加入
    void userLeft(QString usrName,QString time);//处理用户离开
    QString getUsr();//获取用户姓名
    QString getMsg();//获取聊天信息

private:
    QUdpSocket * udpSocket;//udp套接字
    qint16 port;//端口
    QString uName;//用户名

    void ReceiveMessage();//接受UDP消息
};
#endif // WIDGET_H
