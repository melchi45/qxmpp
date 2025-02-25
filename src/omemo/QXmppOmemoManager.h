// SPDX-FileCopyrightText: 2022 Melvin Keskin <melvo@olomono.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPOMEMOMANAGER_H
#define QXMPPOMEMOMANAGER_H

#include "QXmppClientExtension.h"
#include "QXmppE2eeExtension.h"
#include "QXmppMessageHandler.h"
#include "QXmppPubSubEventHandler.h"
#include "QXmppPubSubManager.h"
#include "QXmppTrustSecurityPolicy.h"
#include "qxmppomemo_export.h"

class QXmppOmemoDevicePrivate;
class QXmppOmemoManagerPrivate;
class QXmppOmemoOwnDevicePrivate;
class QXmppOmemoStorage;

class QXMPPOMEMO_EXPORT QXmppOmemoOwnDevice
{
public:
    QXmppOmemoOwnDevice();
    QXmppOmemoOwnDevice(const QXmppOmemoOwnDevice &other);
    QXmppOmemoOwnDevice(QXmppOmemoOwnDevice &&) noexcept;
    ~QXmppOmemoOwnDevice();

    QXmppOmemoOwnDevice &operator=(const QXmppOmemoOwnDevice &);
    QXmppOmemoOwnDevice &operator=(QXmppOmemoOwnDevice &&);

    QString label() const;
    void setLabel(const QString &label);

    QByteArray keyId() const;
    void setKeyId(const QByteArray &keyId);

private:
    QSharedDataPointer<QXmppOmemoOwnDevicePrivate> d;
};

class QXMPPOMEMO_EXPORT QXmppOmemoDevice
{
public:
    QXmppOmemoDevice();
    QXmppOmemoDevice(const QXmppOmemoDevice &other);
    QXmppOmemoDevice(QXmppOmemoDevice &&) noexcept;
    ~QXmppOmemoDevice();

    QXmppOmemoDevice &operator=(const QXmppOmemoDevice &);
    QXmppOmemoDevice &operator=(QXmppOmemoDevice &&);

    QString jid() const;
    void setJid(const QString &jid);

    QString label() const;
    void setLabel(const QString &label);

    QByteArray keyId() const;
    void setKeyId(const QByteArray &keyId);

    QXmpp::TrustLevel trustLevel() const;
    void setTrustLevel(QXmpp::TrustLevel trustLevel);

private:
    QSharedDataPointer<QXmppOmemoDevicePrivate> d;
};

class QXMPPOMEMO_EXPORT QXmppOmemoManager : public QXmppClientExtension, public QXmppE2eeExtension, public QXmppPubSubEventHandler, public QXmppMessageHandler
{
    Q_OBJECT

public:
    using Result = std::variant<QXmpp::Success, QXmppStanza::Error>;

    struct DevicesResult
    {
        QString jid;
        Result result;
    };

    explicit QXmppOmemoManager(QXmppOmemoStorage *omemoStorage);
    ~QXmppOmemoManager() override;

    QFuture<bool> load();
    QFuture<bool> setUp();

    QFuture<QByteArray> ownKey();
    QFuture<QHash<QXmpp::TrustLevel, QMultiHash<QString, QByteArray>>> keys(QXmpp::TrustLevels trustLevels = {});
    QFuture<QHash<QString, QHash<QByteArray, QXmpp::TrustLevel>>> keys(const QList<QString> &jids, QXmpp::TrustLevels trustLevels = {});

    QFuture<bool> changeDeviceLabel(const QString &deviceLabel = {});

    int maximumDevicesPerJid() const;
    void setMaximumDevicesPerJid(int maximum);

    int maximumDevicesPerStanza() const;
    void setMaximumDevicesPerStanza(int maximum);

    QFuture<DevicesResult> requestDeviceLists(const QList<QString> &jids);
    QFuture<DevicesResult> subscribeToDeviceLists(const QList<QString> &jids);
    QFuture<DevicesResult> unsubscribeFromDeviceLists();

    QXmppOmemoOwnDevice ownDevice();
    QFuture<QVector<QXmppOmemoDevice>> devices();
    QFuture<QVector<QXmppOmemoDevice>> devices(const QList<QString> &jids);
    QFuture<QXmppPubSubManager::Result> removeContactDevices(const QString &jid);

    void setAcceptedSessionBuildingTrustLevels(QXmpp::TrustLevels trustLevels);
    QXmpp::TrustLevels acceptedSessionBuildingTrustLevels();

    void setNewDeviceAutoSessionBuildingEnabled(bool isNewDeviceAutoSessionBuildingEnabled);
    bool isNewDeviceAutoSessionBuildingEnabled();

    QFuture<void> buildMissingSessions(const QList<QString> &jids);

    QFuture<bool> resetOwnDevice();
    QFuture<bool> resetAll();

    QFuture<void> setSecurityPolicy(QXmpp::TrustSecurityPolicy securityPolicy);
    QFuture<QXmpp::TrustSecurityPolicy> securityPolicy();

    QFuture<void> setTrustLevel(const QMultiHash<QString, QByteArray> &keyIds, QXmpp::TrustLevel trustLevel);
    QFuture<QXmpp::TrustLevel> trustLevel(const QString &keyOwnerJid, const QByteArray &keyId);

    /// \cond
    QFuture<MessageEncryptResult> encryptMessage(QXmppMessage &&message, const std::optional<QXmppSendStanzaParams> &params) override;
    QFuture<MessageDecryptResult> decryptMessage(QXmppMessage &&message) override;

    QFuture<IqEncryptResult> encryptIq(QXmppIq &&iq, const std::optional<QXmppSendStanzaParams> &params) override;
    QFuture<IqDecryptResult> decryptIq(const QDomElement &element) override;

    bool isEncrypted(const QDomElement &) override;
    bool isEncrypted(const QXmppMessage &) override;

    QStringList discoveryFeatures() const override;
    bool handleStanza(const QDomElement &stanza) override;
    bool handleMessage(const QXmppMessage &message) override;
    /// \endcond

    Q_SIGNAL void trustLevelsChanged(const QMultiHash<QString, QByteArray> &modifiedKeys);

    Q_SIGNAL void deviceAdded(const QString &jid, uint32_t deviceId);
    Q_SIGNAL void deviceChanged(const QString &jid, uint32_t deviceId);
    Q_SIGNAL void deviceRemoved(const QString &jid, uint32_t deviceId);
    Q_SIGNAL void devicesRemoved(const QString &jid);
    Q_SIGNAL void allDevicesRemoved();

protected:
    /// \cond
    void setClient(QXmppClient *client) override;
    bool handlePubSubEvent(const QDomElement &element, const QString &pubSubService, const QString &nodeName) override;
    /// \endcond

private:
    std::unique_ptr<QXmppOmemoManagerPrivate> d;

    friend class QXmppOmemoManagerPrivate;
    friend class tst_QXmppOmemoManager;
};

#endif  // QXMPPOMEMOMANAGER_H
