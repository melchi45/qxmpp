// SPDX-FileCopyrightText: 2019 Linus Jahn <lnj@kaidan.im>
// SPDX-FileCopyrightText: 2010 Jeremy Lainé <jeremy.laine@m4x.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef QXMPPPUBSUBIQ_H
#define QXMPPPUBSUBIQ_H

#include "QXmppIq.h"

#include <optional>

#include <QDomElement>
#include <QSharedDataPointer>

class QXmppDataForm;
class QXmppPubSubIqPrivate;
class QXmppPubSubItem;
class QXmppPubSubSubscription;
class QXmppPubSubAffiliation;
class QXmppResultSetReply;

class QXMPP_EXPORT QXmppPubSubIqBase : public QXmppIq
{
public:
    /// This enum is used to describe a publish-subscribe query type.
    enum QueryType : uint8_t {
        Affiliations,
        OwnerAffiliations,
        Configure,
        Create,
        Default,
        OwnerDefault,
        Delete,
        Items,
        Options,
        Publish,
        Purge,
        Retract,
        Subscribe,
        Subscription,
        Subscriptions,
        OwnerSubscriptions,
        Unsubscribe,
    };

    QXmppPubSubIqBase();
    QXmppPubSubIqBase(const QXmppPubSubIqBase &other);
    ~QXmppPubSubIqBase();

    QXmppPubSubIqBase &operator=(const QXmppPubSubIqBase &other);

    QueryType queryType() const;
    void setQueryType(QueryType queryType);

    QString queryJid() const;
    void setQueryJid(const QString &queryJid);

    QString queryNode() const;
    void setQueryNode(const QString &queryNode);

    QString subscriptionId() const;
    void setSubscriptionId(const QString &subscriptionId);

    QVector<QXmppPubSubSubscription> subscriptions() const;
    void setSubscriptions(const QVector<QXmppPubSubSubscription> &);

    std::optional<QXmppPubSubSubscription> subscription() const;
    void setSubscription(const std::optional<QXmppPubSubSubscription> &);

    QVector<QXmppPubSubAffiliation> affiliations() const;
    void setAffiliations(const QVector<QXmppPubSubAffiliation> &);

    std::optional<uint32_t> maxItems() const;
    void setMaxItems(std::optional<uint32_t>);

    std::optional<QXmppDataForm> dataForm() const;
    void setDataForm(const std::optional<QXmppDataForm> &);

    std::optional<QXmppResultSetReply> itemsContinuation() const;
    void setItemsContinuation(const std::optional<QXmppResultSetReply> &itemsContinuation);

    /// \cond
    static bool isPubSubIq(const QDomElement &element);

protected:
    static bool isPubSubIq(const QDomElement &element,
                           bool (*isItemValid)(const QDomElement &));

    void parseElementFromChild(const QDomElement &) override;
    void toXmlElementFromChild(QXmlStreamWriter *writer) const override;

    virtual void parseItems(const QDomElement &queryElement) = 0;
    virtual void serializeItems(QXmlStreamWriter *writer) const = 0;
    /// \endcond

private:
    static std::optional<QueryType> queryTypeFromDomElement(const QDomElement &element);
    static bool queryTypeIsOwnerIq(QueryType type);

    QSharedDataPointer<QXmppPubSubIqPrivate> d;
};

template<typename T = QXmppPubSubItem>
class QXmppPubSubIq : public QXmppPubSubIqBase
{
public:
    QVector<T> items() const;
    void setItems(const QVector<T> &items);

    static bool isPubSubIq(const QDomElement &element);

protected:
    /// \cond
    void parseItems(const QDomElement &queryElement) override;
    void serializeItems(QXmlStreamWriter *writer) const override;
    /// \endcond

private:
    QVector<T> m_items;
};

template<typename T>
QVector<T> QXmppPubSubIq<T>::items() const
{
    return m_items;
}

template<typename T>
void QXmppPubSubIq<T>::setItems(const QVector<T> &items)
{
    m_items = items;
}

template<typename T>
bool QXmppPubSubIq<T>::isPubSubIq(const QDomElement &element)
{
    return QXmppPubSubIqBase::isPubSubIq(element, [](const QDomElement &item) -> bool {
        return T::isItem(item);
    });
}

/// \cond
template<typename T>
void QXmppPubSubIq<T>::parseItems(const QDomElement &queryElement)
{
    for (auto childElement = queryElement.firstChildElement(QStringLiteral("item"));
         !childElement.isNull();
         childElement = childElement.nextSiblingElement(QStringLiteral("item"))) {
        T item;
        item.parse(childElement);
        m_items << std::move(item);
    }
}

template<typename T>
void QXmppPubSubIq<T>::serializeItems(QXmlStreamWriter *writer) const
{
    for (const auto &item : std::as_const(m_items)) {
        item.toXml(writer);
    }
}
/// \endcond

#endif  // QXMPPPUBSUBIQ_H
