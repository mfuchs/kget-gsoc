/* This file is part of the KDE project

   Copyright (C) 2005 Dario Massarin <nekkar@libero.it>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; version 2
   of the License.
*/

#include <kdebug.h>

#include "core/job.h"
#include "core/jobqueue.h"
#include "core/scheduler.h"

Job::Job(JobQueue * parent, Scheduler * scheduler)
    : m_jobQueue(parent),
      m_scheduler(scheduler),
      m_status(Stopped),
      m_policy(None)
{

}

Job::~Job()
{
}

void Job::setStatus(Status jobStatus)
{
    if(jobStatus == m_status)
        return;

    if(m_status == Job::Delayed)
    {
        //The previous status was Job::Delayed. We must stop all the timers
        m_scheduler->stopDelayTimer(this);
    }

    m_status = jobStatus;
    m_scheduler->jobChangedEvent(this, m_status);
}

void Job::setPolicy(Policy jobPolicy)
{
    if(jobPolicy == m_policy)
        return;

    kDebug(5001) << "Job::setPolicy(" << jobPolicy << ")" << endl;

    m_policy = jobPolicy;
    m_scheduler->jobChangedEvent(this, m_policy);
}
