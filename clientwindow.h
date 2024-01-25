#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QMainWindow>
#include <memory>
#include <vector>
#include <iterator>
#include "Database.h"

namespace Ui {
class ClientWindow;
}

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientWindow(int userID, std::shared_ptr<Database> dbPtr = nullptr, bool visible = false, QWidget *parent = nullptr);
    ~ClientWindow();

    static ClientWindow* createClient(std::shared_ptr<Database> dbPtr = nullptr);
    static ClientWindow* createAdministration(std::shared_ptr<Database> dbPtr = nullptr);
    std::string getUserName();
    void setPossibility(bool set);
    void closeClient();
    int getUserIDDialog();
    void addClient(ClientWindow* w);
    static int kInstanceCount;
    static std::vector<ClientWindow*> clientList;

private slots:
    void updateChats();
    void on_SendMessage_clicked();
    void on_PrivateMessage_clicked();
    void on_MessageEdit_returnPressed();
    void on_BanClient_clicked();
    void on_CloseClient_clicked();
    void on_CreatNewWindow_triggered();
    void on_LoginAdministration_triggered();
    void on_Quit_triggered();
    int getUserDialog(std::vector<std::string> listUsers);

private:
    Ui::ClientWindow *ui;
    int m_userID;
    QString m_userName;
    std::vector<ClientWindow*>::iterator m_w;
    std::shared_ptr<Database> m_dbPtr;
    //std::vector<ClientWindow*> m_clientList;

};

#endif // CLIENTWINDOW_H
