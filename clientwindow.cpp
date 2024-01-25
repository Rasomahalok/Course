#include "clientwindow.h"
#include "ui_clientwindow.h"
#include "startscreen.h"
#include <QTimer>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QListWidget>
#include <vector>

int ClientWindow::kInstanceCount = 0;
std::vector<ClientWindow*> ClientWindow::clientList;

ClientWindow::ClientWindow(int userID, std::shared_ptr<Database> dbPtr, bool visible, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClientWindow),
    m_userID(userID)
{
    ui->setupUi(this);
    kInstanceCount++;
    if (dbPtr) m_dbPtr = dbPtr;
    else m_dbPtr = make_shared<Database>();
    m_userName = QString::fromStdString(m_dbPtr->getUserName(m_userID));
    ui->BanClient->setVisible(visible);
    ui->CloseClient->setVisible(visible);
    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &ClientWindow::updateChats);
    timer->start(10);
}

ClientWindow::~ClientWindow()
{
    delete ui;
    if (m_w >= clientList.begin() && m_w < clientList.end()) clientList.erase(m_w);
    kInstanceCount--;
    if (kInstanceCount <= 0) qApp->exit(0);
}

void ClientWindow::addClient(ClientWindow* w)
{
    clientList.push_back(w);
    m_w = clientList.end() - 1;
}

ClientWindow* ClientWindow::createClient(std::shared_ptr<Database> dbPtr)
{
    StartScreen s(dbPtr);
    auto result = s.exec();
    if (result == QDialog::Rejected) return nullptr;
    auto w = new ClientWindow(s.userId(), s.getDatabase(), false);
    w->setAttribute(Qt::WA_DeleteOnClose);
    return w;
}

ClientWindow* ClientWindow::createAdministration(std::shared_ptr<Database> dbPtr)
{
    StartScreen s(dbPtr, false);
    auto result = s.exec();
    if (result == QDialog::Rejected) return nullptr;
    auto w = new ClientWindow(s.userId(), s.getDatabase(), true);
    w->setAttribute(Qt::WA_DeleteOnClose);
    return w;
}

void ClientWindow::on_SendMessage_clicked()
{
    m_dbPtr->addChatMessage(m_userName.toStdString(), ui->MessageEdit->text().toStdString());
    ui->MessageEdit->clear();
}

int ClientWindow::getUserDialog(std::vector<std::string> listUsers)
{
    QDialog dlg(this);
    dlg.setModal(true);
    auto l = new QVBoxLayout();
    dlg.setLayout(l);
    auto userListWgt = new QListWidget(&dlg);
    l->addWidget(userListWgt);
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    l->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    for (const auto &user : listUsers) userListWgt->addItem(QString::fromStdString(user));
    userListWgt->setCurrentRow(0);
    auto result = dlg.exec();
    if (result == QDialog::Accepted && userListWgt->currentItem()) return userListWgt->currentRow();
    return -1;
}

void ClientWindow::on_PrivateMessage_clicked()
{
    auto userList = m_dbPtr->getUserList();
    auto userID = getUserDialog(userList);
    if (userID != -1) {
        m_dbPtr->addPrivateMessage(m_userName.toStdString(), userList.at(userID), ui->MessageEdit->text().toStdString());
        ui->MessageEdit->clear();
    }
}

void ClientWindow::on_MessageEdit_returnPressed()
{
    Qt::KeyboardModifiers keyMod = QApplication::keyboardModifiers();
    if (keyMod.testFlag(Qt::ShiftModifier)) on_PrivateMessage_clicked();
    else on_SendMessage_clicked();
}

void ClientWindow::updateChats()
{
    auto chatMessages = m_dbPtr->getChatMessages();
    QString chat;
    for (const auto &msg : chatMessages) chat.append(QString::fromStdString(msg) + '\n');
    if (ui->AllMessages->toPlainText() != chat) ui->AllMessages->setText(chat);

    chat.clear();
    auto privateMessages = m_dbPtr->getPrivateMessage();
    if (ui->BanClient->isVisible())
        for (const auto &msg : privateMessages)
            chat.append(QString("<%1> шепчет <%2>: %3\n").arg(QString::fromStdString(msg.getSender())).arg(QString::fromStdString(m_dbPtr->getUserName(msg.getDest()))).arg(QString::fromStdString(msg.getText())));
    else for (const auto &msg : privateMessages) {
        if (msg.getSender() != m_dbPtr->getUserName(m_userID) && msg.getDest() != m_userID) continue;
        QString prefix;
        if (msg.getSender() == m_dbPtr->getUserName(m_userID) && msg.getDest() == m_userID) prefix = "Себе: ";
        else if (msg.getSender() == m_dbPtr->getUserName(m_userID)) prefix = QString("Сообщение <%1>: ").arg(QString::fromStdString(m_dbPtr->getUserName(msg.getDest())));
        else prefix = QString("<%1> пишет тебе: ").arg(m_userName);
        chat.append(prefix + QString::fromStdString(msg.getText()) + '\n');
    }
    if (ui->PrivateMessages->toPlainText() != chat) ui->PrivateMessages->setText(chat);
}

std::string ClientWindow::getUserName()
{
    return m_userName.toStdString();
}

void ClientWindow::setPossibility(bool set)
{
    ui->SendMessage->setEnabled(set);
}

void ClientWindow::closeClient()
{
    this->close();
}

int ClientWindow::getUserIDDialog()
{
    std::vector<std::string> userList;
    for (const auto &client : clientList) userList.push_back(client->getUserName());
    return getUserDialog(userList);
}

void ClientWindow::on_BanClient_clicked()
{
    auto userID = getUserIDDialog();
    if (userID != -1) clientList.at(userID)->setPossibility(false);
}

void ClientWindow::on_CloseClient_clicked()
{
    auto userID = getUserIDDialog();
    if (userID != -1) clientList.at(userID)->closeClient();
}

void ClientWindow::on_CreatNewWindow_triggered()
{
    auto w = createClient(m_dbPtr);
    if (w) {
        w->addClient(w);
        w->show();
    }
}

void ClientWindow::on_LoginAdministration_triggered()
{
    auto w = createAdministration(m_dbPtr);
    if (w) w->show();
}

void ClientWindow::on_Quit_triggered()
{
    this->close();
}
