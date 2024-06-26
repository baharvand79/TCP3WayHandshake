#ifndef PACKET_H
#define PACKET_H

#include <QByteArray>
#include <QDataStream>
#include <QIODevice>

struct Packet {
    quint8 flags;
    quint32 sequenceNumber;
    quint16 maxSegmentSize;
    quint16 windowSize;
    QByteArray data;
    QByteArray checksum;

    Packet(quint8 f = 0, quint32 seq = 0, quint16 maxSegSize = 0, quint16 winSize = 0, const QByteArray& d = QByteArray(), const QByteArray& ch = QByteArray())
        : flags(f), sequenceNumber(seq), maxSegmentSize(maxSegSize), windowSize(winSize), data(d), checksum(ch)
    {
    }

    // Serialization method
    QByteArray serialize() const {
        QByteArray serializedData;
        QDataStream stream(&serializedData, QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_5_15);
        stream << flags << sequenceNumber << maxSegmentSize << windowSize << data << checksum;
        return serializedData;
    }

    // Deserialization method
    static Packet deserialize(const QByteArray& serializedData) {
        QDataStream stream(serializedData);
        stream.setVersion(QDataStream::Qt_5_15);
        quint8 flags;
        quint32 sequenceNumber;
        quint16 maxSegmentSize;
        quint16 windowSize;
        QByteArray data;
        QByteArray checksum;
        stream >> flags >> sequenceNumber >> maxSegmentSize >> windowSize >> data >> checksum;
        return Packet(flags, sequenceNumber, maxSegmentSize, windowSize, data, checksum);
    }
};

#endif // PACKET_H
