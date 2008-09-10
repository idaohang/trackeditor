/*
 * CWintec.cpp
 *
 *  Created on: 26.08.2008
 *      Author: martin
 */

#include <QDebug>
#include "CWintec.h"
#include "DeviceData.h"

CWintec::CWintec(QString name) : IDeviceIO(name) , m_num_retries(5){
	qDebug() << QString("CWintec::WCintec name: %1").arg(name);

	m_dev_data = 0;
	m_expect_binary_data = 0;

	m_command_mode_step = 0;
	m_command_response_step = 0;

    m_step_complete = true;
    m_retry_count = m_num_retries;
    m_timer = new QTimer();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));

}

CWintec::~CWintec() {
    if(m_timer->isActive()) {
        m_timer->stop();
    }
    delete m_timer;
}


QStringList CWintec::getInterfaceList() {

	QStringList stringlist;
	stringlist.append("usb");
	stringlist.append("bluetooth");
	return stringlist;
}


void CWintec::addData(QByteArray data) {

	if(m_expect_binary_data == 0) {
		m_nema_string.append(data);
		do {
			m_line = readLine();
			if (m_line.size() != 0) {
				// qDebug() << QString("setLine: %1 !end!").arg(m_line);
				emit nemaString(m_line);
				parseLine(m_line);

				enterCommandMode(m_command_mode_step);
			}
		} while(m_line.size() != 0);
	}
	else {  // m_expect_binary_data > 0 -> read m_expect_binary_data bytes of binary data

		if(data.size() <= m_expect_binary_data) {
			m_tmp_buf.append(data);
			m_expect_binary_data -= data.size();

			// memcpy(buffer + m_binary_data_already_read, data.constData(), data.size());
			// m_binary_data_already_read += bytes_read;
		}
		else { // if more data that expected, split into binary and text.
			qDebug() << "Error: more binary data than expected was received.";
		}

		//bytes_read = read(m_device_fd, buffer + m_binary_data_already_read, m_expect_binary_data);
		//m_binary_data_already_read += bytes_read;
		//m_expect_binary_data -= bytes_read;
		//if(m_expect_binary_data == 0){
			// all expected binary data was read into buffer.
			// store data in m_tmp_buffer which is used to calc checksum and
			// will be appended to m_log_buf if checksum was correct.
		//	m_tmp_buf = QByteArray(buffer, m_binary_data_already_read);
		//}
	}


}

bool CWintec::readLog() {
	if (m_command_mode_step != 0) {
		qDebug("already in command mode.");
		return false;
	}
	m_command_mode_step = 1;

	m_dev_data = new DeviceData();

    enterCommandMode(m_command_mode_step);
	return true;
}

void CWintec::leaveCommandMode()
{
	m_expect_binary_data = 0;
	m_command_mode_step = 0;
	m_command_response_step = 0;
	// Exit command mode

    write(m_device_fd, "@AL,02,01\n", strlen("@AL,02,01\n"));
}


QString CWintec::readLine() {
	int pos;

	QString tmp_string;

	pos = m_nema_string.indexOf("\n");

	if (pos == -1) {
		// no end of line found in string. Line not complete yet.
		// return empty String and wait for next invocation.
		tmp_string.clear();
		return tmp_string;
	}

	tmp_string = m_nema_string.left(pos);
	m_nema_string = m_nema_string.right(m_nema_string.length() - pos - 1);

	int last_pos = tmp_string.lastIndexOf("\r");
	if(last_pos != -1)
	{
		tmp_string.chop(tmp_string.size() - last_pos );
	}
	return tmp_string;
}


void CWintec::timeout() {
    if(m_retry_count == 0) {
        // fatal error
        emit readLogFailed("Timeout reading from device!");
        leaveCommandMode();
    }
    m_retry_count--;
    m_step_complete = true;
    enterCommandMode(m_command_mode_step);
}


void CWintec::enterCommandMode(int step) {

	if(m_step_complete == false) {
		// the command mode step has not finished yet
		// but this function is triggered by any incoming data.
		// Only enter a step and send commands to device if previous step is complete.
		// A timeout on waiting for a response will reset the step to finished, too.
		return;
	}

	switch (step) {
	case 1:
		qDebug("enterCommandMode step %d.", step);
		qDebug("send @AL,02,01\n");
		emit sendData(QString("@AL,02,01\n").toUtf8());
		m_step_complete = false;
		// wait for readLine to finish ...
		break;

	case 2:
        qDebug("enterCommandMode step %d.", step);
        qDebug("send @AL\n");
        qDebug("@AL,2,3\n");
        qDebug("@AL,2,3\n");

        emit sendData(QString("@AL\n").toUtf8());
        emit sendData(QString("@AL,2,3\n").toUtf8());
        emit sendData(QString("@AL,2,3\n").toUtf8());
        //write(m_device_fd, "@AL\n", strlen("@AL\n"));
		//write(m_device_fd, "@AL,2,3\n", strlen("@AL,2,3\n"));
		//write(m_device_fd, "@AL,2,3\n", strlen("@AL,2,3\n"));

        // m_command_mode_step++;
        m_step_complete = false;
		break;

	case 3:
		//if(m_command_response_step < 2)
		//{
			// LoginOK not yet received
			//break;
		//}
        qDebug("enterCommandMode step %d.", step);
        emit sendData(QString("@AL,07,01\n").toUtf8());
        m_step_complete = false;
        // m_command_mode_step++;
		break;

    case 4:
    	qDebug("enterCommandMode step %d.", step);
        emit sendData(QString("@AL,07,02\n").toUtf8());
    	// m_command_mode_step++;
        m_step_complete = false;
    	break;

    case 5:
        qDebug("enterCommandMode step %d.", step);
        emit sendData(QString("@AL,07,03\n").toUtf8());
        // m_command_mode_step++;
        m_step_complete = false;
        break;

    case 6:
        qDebug("enterCommandMode step %d. => @AL,05,01", step);
        emit sendData(QString("@AL,05,01\n").toUtf8());
        // write(m_device_fd, "@AL,05,01\n", strlen("@AL,05,01\n"));
        // m_command_mode_step++;
        m_step_complete = false;
        break;

    case 7:
        qDebug("enterCommandMode step %d. => @AL,05,02", step);
        emit sendData(QString("@AL,05,02\n").toUtf8());
        // m_command_mode_step++;
        m_step_complete = false;
        break;

    case 8:
        qDebug("enterCommandMode step %d. => @AL,05,09", step);
        emit sendData(QString("@AL,05,09\n").toUtf8());
        // m_command_mode_step++;
        m_step_complete = false;
        break;

    case 9:
        qDebug("enterCommandMode step %d. => @AL,05,10", step);
        emit sendData(QString("@AL,05,10\n").toUtf8());
        // m_command_mode_step++;
        m_step_complete = false;
        break;

    default:
    	break;
	}
}

void CWintec::parseLine(QString line) {
	// qDebug() << QString("parseLine: ") << line;

	if (m_command_mode_step == 0) {
		parseNEMA(line);
	} else {
		qDebug() << QString("parseLine: ") << line;
		parseAL(line);
	}
}

void CWintec::parseAL(QString line) {

    // expect the responses from the set to command mode state machine.
	// wait for LoginOK. If received, switchng to command mode was successful
	// and we can continue reading out data.

    QString expectstr("@AL,02,01");
    //qDebug() << QString("expecting line %1: %2").arg(m_command_response_step).arg(expectstr);
    if (expectstr.compare(line) == 0) {
            qDebug() << QString("got %1 proceeding... ").arg(expectstr);
            m_command_mode_step = 2;
            m_step_complete = true;
            return;
    }

    expectstr = QString("@AL,LoginOK");
    //qDebug() << QString("expecting line %1: %2").arg(m_command_response_step).arg(expectstr);
    if (expectstr.compare(line) == 0) {
            qDebug("got LoginOK, proceeding... ");
            m_command_mode_step = 3;
            m_step_complete = true;
            return;
    }


    // get device name (usually WBT201)
    expectstr = QString("@AL,07,01,");
    if (line.startsWith(expectstr)) {
    	QString model = line.section(',', 3, 3);
        qDebug() << QString("model = %1").arg(model);
        m_dev_data->setDeviceName(model);
        m_command_mode_step = 4;
        m_step_complete = true;
        return;
    }

    // get device info
    expectstr = QString("@AL,07,02,");
    if (line.startsWith(expectstr)) {
        QString info = line.section(',', 3, 3);
        qDebug() << QString("device info = %1.").arg(info);
        m_dev_data->setDeviceInfo(info);
        m_command_mode_step = 5;
        m_step_complete = true;
        return;
    }

    // get device serial No
    expectstr = QString("@AL,07,03,");
    if (line.startsWith(expectstr)) {
        QString serial = line.section(',', 3, 3);
        qDebug() << QString("device serial No = %1.").arg(serial);
        m_dev_data->setDeviceSerial(serial);
        m_command_mode_step = 6;
        m_step_complete = true;
        return;
    }

    // get log start address
    expectstr = QString("@AL,05,01,");
    if (line.startsWith(expectstr)) {
        QString log_start_str = line.section(',', 3, 3);
        long logstart = log_start_str.toLong();
        qDebug() << QString("Log start = %1. As int %2").arg(log_start_str).arg(logstart);
        m_dev_data->setLogStart(logstart);
        m_command_mode_step = 7;
        m_step_complete = true;
        if(m_dev_data->complete()) {
        	readLogMode();
        }
        return;
    }

    // get log end address
    expectstr = QString("@AL,05,02,");
    if (line.startsWith(expectstr)) {
        QString log_end_str = line.section(',', 3, 3);
        long logend = log_end_str.toLong();
        qDebug() << QString("Log end = %1. As int %2").arg(log_end_str).arg(logend);
        m_dev_data->setLogEnd(logend);
        m_command_mode_step = 8;
        m_step_complete = true;
        if(m_dev_data->complete()) {
        	readLogMode();
        }
        return;
    }

    // get log area start address
    expectstr = QString("@AL,05,09,");
    if (line.startsWith(expectstr)) {
        QString log_area_start_str = line.section(',', 3, 3);
        long logareastart = log_area_start_str.toLong();
        qDebug() << QString("Log area start = %1. As int %2").arg(log_area_start_str).arg(logareastart);
        m_dev_data->setLogAreaStart(logareastart);
        m_command_mode_step = 9;
        m_step_complete = true;
        if(m_dev_data->complete()) {
        	readLogMode();
        }
        return;
    }

    // get log area end address
    expectstr = QString("@AL,05,10,");
    //qDebug() << QString("expecting line %1: %2").arg(m_command_response_step).arg(expectstr);
    if (line.startsWith(expectstr)) {
        QString log_area_end_str = line.section(',', 3, 3);
        long logareaend = log_area_end_str.toLong();
        qDebug() << QString("Log area end = %1. As int %2").arg(log_area_end_str).arg(logareaend);
        m_dev_data->setLogAreaEnd(logareaend);
        m_command_mode_step = 10;
        m_step_complete = true;
        if(m_dev_data->complete()) {
        	readLogMode();
        }
        return;
    }

    // get checksum over received binary data
    expectstr = QString("@AL,CS,");
    //qDebug() << QString("expecting line %1: %2").arg(m_command_response_step).arg(expectstr);
    if (line.startsWith(expectstr)) {
        QString checksum_str = line.section(',', 2, 2);
        qDebug() << QString("checksum string = %1.").arg(checksum_str);
        // if(checksumCorrect) {
        m_log_buf.append(m_tmp_buf);
        m_retry_count = 5;

        if(m_lastsection == 1 || m_tmp_buf.size() == 0) {
        	createTrackpoints();
        	leaveCommandMode();
            return;
        }
        m_read_start += m_tmp_buf.size();

        readLogData();
        return;
    }

}

void CWintec::readLogMode() {
    m_read_start = m_dev_data->getLogStart();
    m_retry_count = 5;
    m_lastsection = false;
    m_blocksize = 4096;

    m_command_response_step = 3;

    m_log_buf.clear();
    m_log_buf.reserve((m_dev_data->getLogAreaEnd() - m_dev_data->getLogAreaStart()));

    readLogData();
}

void CWintec::readLogData() {

	int logstart = m_dev_data->getLogStart();
	int logend = m_dev_data->getLogEnd();

	int logareastart = m_dev_data->getLogAreaStart();
	int logareaend = m_dev_data->getLogAreaEnd();

    qDebug() << QString("Logarea: %1 - %2").arg(logareastart).arg(logareaend);
    qDebug() << QString("Log: %1 - %2").arg(logstart).arg(logend);

    int trackpointlen = 16;
    int log_capacy = (m_dev_data->getLogAreaEnd() - m_dev_data->getLogAreaStart())
				     / trackpointlen;

    if (log_capacy < m_blocksize) {
    	m_expect_binary_data = log_capacy;
        m_lastsection = true;
    }
    else if(logstart + m_blocksize >= logend) {
    	m_expect_binary_data = logend - logstart;
        m_lastsection = true;
    }
    else if(m_read_start + m_blocksize >= logend) {
        m_expect_binary_data = logend - m_read_start;
        m_lastsection = true;
        qDebug() << QString("re3adin last rest at %1 length %2.").arg(m_read_start).arg(logend - m_read_start);
    }
    else {
    	m_expect_binary_data = m_blocksize;
    }
//    if(m_read_start >= m_dev_data->getLogAreaEnd()) {
//        m_read_start = m_dev_data->getLogAreaStart();
//    }
//    if (m_read_start < m_dev_data->getLogEnd() &&
//            m_read_start + m_blocksize >= m_dev_data->getLogEnd()) {
//        m_read_count = m_dev_data->getLogEnd() - m_read_start;
//        m_lastsection = 1
//    }


    char cmd_buf[32];
    snprintf(cmd_buf, 32, "@AL,05,03,%d\n", m_read_start);
    m_binary_data_already_read = 0;
    m_tmp_buf.clear();

    write(m_device_fd, cmd_buf, strlen(cmd_buf));

}

void CWintec::createTrackpoints() {
	FILE* fd;
	Track* track = 0;
	fd = fopen("outfile2.tkraw", "wb");
    qDebug() << QString("createTrackpoints()");
    int trackindex = 0;

    // m_track_collection->clear();

	for(int pos = 0; pos < m_log_buf.size(); pos += TrackPoint::size()) {
		fwrite(m_log_buf.mid(pos, TrackPoint::size()).data(), m_log_buf.mid(pos, TrackPoint::size()).size(), 1, fd);
		TrackPoint* tp = new TrackPoint(m_log_buf.mid(pos, TrackPoint::size()));


		if(tp->isLogPoint()) {
			emit newLogPoint(tp);
		}
		else {
			if(tp->isBeginOfTrack()) {
				if(track != 0) {
					track->commit();
					emit newTrack(track);
					qDebug() << QString("End of Track (%1 - %2 , %3 - %4)").arg(track->getMinLong()).arg(track->getMaxLong()).arg(track->getMinLat()).arg(track->getMaxLat());
				}
				track = new Track();
				track->setIndex(trackindex);
				trackindex++;
				//m_track_collection->append(track);
			}
			track->append(tp);
		}

	}
	track->commit();
	emit newTrack(track);
//	m_track_view->setTrackColletcion(m_track_collection);
//	m_track_collection->commit();

//	ui.treeView->resizeColumnToContents(0);
//	ui.treeView->resizeColumnToContents(1);
//	ui.treeView->resizeColumnToContents(2);
//	ui.treeView->resizeColumnToContents(3);
//	fclose(fd);

//	m_track_view->update();
}


void CWintec::parseNEMA(QString line) {
	// qDebug() << line;
}
