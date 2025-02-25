// SPDX-FileCopyrightText: 2020 Linus Jahn <lnj@kaidan.im>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "QXmppTuneItem.h"

#include "QXmppConstants_p.h"
#include "QXmppUtils.h"

#include <QDomElement>
#include <QUrl>
#include <QXmlStreamWriter>

/// \cond
class QXmppTuneItemPrivate : public QSharedData
{
public:
    QXmppTuneItemPrivate();

    QString artist;
    std::optional<quint16> length;
    std::optional<quint8> rating;
    QString source;
    QString title;
    QString track;
    QUrl uri;
};

QXmppTuneItemPrivate::QXmppTuneItemPrivate()
    : length(0),
      rating(0)
{
}
/// \endcond

///
/// \class QXmppTuneItem
///
/// This class represents a PubSub item for \xep{0118, User Tune}.
///
/// \since QXmpp 1.5
///

///
/// Default constructor
///
QXmppTuneItem::QXmppTuneItem()
    : d(new QXmppTuneItemPrivate)
{
}

/// Copy-constructor.
QXmppTuneItem::QXmppTuneItem(const QXmppTuneItem &other) = default;
/// Move-constructor.
QXmppTuneItem::QXmppTuneItem(QXmppTuneItem &&) = default;
QXmppTuneItem::~QXmppTuneItem() = default;
/// Assignment operator.
QXmppTuneItem &QXmppTuneItem::operator=(const QXmppTuneItem &other) = default;
/// Move-assignment operator.
QXmppTuneItem &QXmppTuneItem::operator=(QXmppTuneItem &&) = default;

///
/// Returns the artist of the piece or song.
///
QString QXmppTuneItem::artist() const
{
    return d->artist;
}

///
/// Sets the artist of the piece or song.
///
void QXmppTuneItem::setArtist(QString artist)
{
    d->artist = std::move(artist);
}

///
/// Returns the length of the piece in seconds (0 means unknown).
///
std::optional<quint16> QXmppTuneItem::length() const
{
    return d->length;
}

///
/// Sets the length of the piece in seconds (0 means unknown).
///
void QXmppTuneItem::setLength(std::optional<quint16> length)
{
    d->length = length;
}

///
/// \fn QXmppTuneItem::lengthAsTime()
///
/// Returns the length as QTime.
///

///
/// \fn QXmppTuneItem::setLength(const QTime &time)
///
/// Sets the length from QTime.
///
/// \overload
///

///
/// \fn QXmppTuneItem::lengthAsDuration()
///
/// Returns the length as std::chrono::seconds.
///
/// \overload
///

///
/// \fn QXmppTuneItem::setLength(std::optional<std::chrono::seconds> time)
///
/// Sets the length from std::chrono::seconds. Useful if you want to use the
/// chrono literals.
///
/// \overload
///

///
/// Returns the user's rating of the song or piece (from 1 to 10), 0 means
/// invalid or unknown.
///
std::optional<quint8> QXmppTuneItem::rating() const
{
    return d->rating;
}

///
/// Sets the user's rating of the song or piece (from 1 to 10), 0 means invalid
/// or unknown.
///
void QXmppTuneItem::setRating(std::optional<quint8> rating)
{
    if (rating) {
        if (auto r = *rating; r <= 10 && r != 0) {
            d->rating = rating;
            return;
        }
    }
    d->rating.reset();
}

///
/// Returns the album, other collection or other source (e.g. website) of the
/// piece.
///
QString QXmppTuneItem::source() const
{
    return d->source;
}

///
/// Sets the album, other collection or other source (e.g. website) of the
/// piece.
///
void QXmppTuneItem::setSource(QString source)
{
    d->source = std::move(source);
}

///
/// Returns the title of the piece.
///
QString QXmppTuneItem::title() const
{
    return d->title;
}

///
/// Sets the title of the piece.
///
void QXmppTuneItem::setTitle(QString title)
{
    d->title = std::move(title);
}

///
/// Returns the track number or other identifier in the collection or source.
///
QString QXmppTuneItem::track() const
{
    return d->track;
}

///
/// Sets the track number or other identifier in the collection or source.
///
void QXmppTuneItem::setTrack(QString track)
{
    d->track = std::move(track);
}

///
/// Returns an URI or URL pointing to information about the song, collection or
/// artist.
///
QUrl QXmppTuneItem::uri() const
{
    return d->uri;
}

///
/// Sets an URI or URL pointing to information about the song, collection or
/// artist.
///
void QXmppTuneItem::setUri(QUrl uri)
{
    d->uri = std::move(uri);
}

///
/// Returns true, if the element is a valid \xep{0118}: User Tune PubSub item.
///
bool QXmppTuneItem::isItem(const QDomElement &itemElement)
{
    auto isPayloadValid = [](const QDomElement &payload) -> bool {
        return payload.tagName() == QStringLiteral("tune") &&
            payload.namespaceURI() == ns_tune;
    };

    return QXmppPubSubItem::isItem(itemElement, isPayloadValid);
}

/// \cond
void QXmppTuneItem::parsePayload(const QDomElement &tune)
{
    for (auto child = tune.firstChildElement();
         !child.isNull();
         child = child.nextSiblingElement()) {
        if (child.tagName() == QStringLiteral("artist")) {
            d->artist = child.text();
        } else if (child.tagName() == QStringLiteral("length")) {
            bool ok = false;
            if (auto len = child.text().toUShort(&ok); ok) {
                d->length = len;
            }
        } else if (child.tagName() == QStringLiteral("rating")) {
            bool ok = false;
            if (auto len = child.text().toUShort(&ok); ok) {
                setRating(len);
            }
        } else if (child.tagName() == QStringLiteral("source")) {
            d->source = child.text();
        } else if (child.tagName() == QStringLiteral("title")) {
            d->title = child.text();
        } else if (child.tagName() == QStringLiteral("track")) {
            d->track = child.text();
        } else if (child.tagName() == QStringLiteral("uri")) {
            d->uri = QUrl(child.text());
        }
    }
}

void QXmppTuneItem::serializePayload(QXmlStreamWriter *writer) const
{
    writer->writeStartElement(QStringLiteral("tune"));
    writer->writeDefaultNamespace(ns_tune);

    helperToXmlAddTextElement(writer, QStringLiteral("artist"), d->artist);
    if (d->length) {
        writer->writeTextElement(QStringLiteral("length"), QString::number(*d->length));
    }
    if (d->rating) {
        writer->writeTextElement(QStringLiteral("rating"), QString::number(*d->rating));
    }
    helperToXmlAddTextElement(writer, QStringLiteral("source"), d->source);
    helperToXmlAddTextElement(writer, QStringLiteral("title"), d->title);
    helperToXmlAddTextElement(writer, QStringLiteral("track"), d->track);
    helperToXmlAddTextElement(writer, QStringLiteral("uri"), d->uri.toString(QUrl::FullyEncoded));

    writer->writeEndElement();
}
/// \endcond
