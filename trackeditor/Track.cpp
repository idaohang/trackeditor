/*
 * Track.cpp
 *
 *  Created on: 03.08.2008
 *      Author: martin
 */

#include "Track.h"

Track::Track() {
	initMetaData();

//	m_itemlist = new QList<QStandardItem*>;
}

Track::~Track() {
	for(int i = 0; i < m_itemlist.size(); i++) {
		delete m_itemlist.at(i);
	}
	//delete m_itemlist;
}

void Track::commit() {
	// iterate over QVector and find min and max lat and long
	m_min_lat = 360.0;
	m_max_lat = 0;
	m_min_lng = 360.0;
	m_max_lng = 0;

	if(size() == 0) return;

	m_min_time = at(0)->getTime();
	m_max_time = at(0)->getTime();
	int i;

	for(i = 0; i < size(); i++) {
		if(m_min_lat > at(i)->getLat() ) m_min_lat = at(i)->getLat();
		if(m_max_lat < at(i)->getLat() ) m_max_lat = at(i)->getLat();
		if(m_min_lng > at(i)->getLong() ) m_min_lng = at(i)->getLong();
		if(m_max_lng < at(i)->getLong() ) m_max_lng = at(i)->getLong();

		if(m_min_time > at(i)->getTime()) m_min_time = at(i)->getTime();
		if(m_max_time < at(i)->getTime()) m_max_time = at(i)->getTime();
	}

	QStandardItem* item;

	QString tracknr;
	tracknr.setNum(m_track_index);
	item = new QStandardItem(tracknr);
	m_itemlist.append(item);

	QString starttime = m_min_time.toString("yy-MM-dd  hh:mm:ss");
	item = new QStandardItem(starttime);
	m_itemlist.append(item);

	QString endtime = m_max_time.toString("yy-MM-dd  hh:mm:ss");
	item = new QStandardItem(endtime);
	m_itemlist.append(item);

	QString nrpoints;
	nrpoints.setNum(size());
	item = new QStandardItem(nrpoints);
	m_itemlist.append(item);

}

void Track::initMetaData() {
	m_min_lat = 0;
	m_max_lat = 0;
	m_min_lng = 0;
	m_max_lng = 0;

	m_track_index = 0;

//	m_track_name.clear();
//	m_track_comment.clear();
//	m_track_description.clear();
//	m_track_data_src.clear();
//	m_track_link.clear();
//	m_track_type.clear();

}

double Track::getMinLat() {
	return m_min_lat;
}

double Track::getMaxLat() {
	return m_max_lat;
}

double Track::getMinLong() {
	return m_min_lng;
}

double Track::getMaxLong() {
	return m_max_lng;
}

QDateTime Track::getTrackBegin() {
	return m_min_time;
}

QDateTime Track::getTrackEnd() {
	return m_max_time;
}

void Track::setIndex(int index) {
	m_track_index = index;
}

int Track::getIndex(void) {
	return m_track_index;
}

bool Track::validIndex(void) {
	if(m_track_index >= 0) {
		return true;
	}
	else {
		return false;
	}
}

QList<QStandardItem*> Track::getItemList() {
	return m_itemlist;
}

//
//void Track::setTrackName(QString name) {
//	m_track_name = name;
//}
//
//QString Track::getTrackName(void) {
//	return m_track_name;
//}
//
//
//void Track::setTrackDescription(QString desc) {
//	m_track_description = desc;
//}
//
//QString Track::getTrackDescription(void) {
//	return m_track_description;
//}
//
//
//
//void Track::setTrackLink(QString link) {
//	m_track_link = link;
//}
//
//QString Track::getTrackLink(void) {
//	return m_track_link;
//}
//