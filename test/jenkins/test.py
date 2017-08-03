#!/usr/bin/python

#pip install --user jenkinsapi
#https://jenkinsapi.readthedocs.io/en/latest/using_jenkinsapi.html
from jenkinsapi.jenkins import Jenkins

""" Get version of Jenkins"""
def get_server_instance():
    jenkins_url = 'http://localhost:8080'
    server = Jenkins(jenkins_url, username='unuser', password='unpass')
    return server

"""Get job details of each job that is running on the Jenkins instance"""
def get_job_details():
    # Refer Example #1 for definition of function 'get_server_instance'
    server = get_server_instance()
    for job_name, job_instance in server.get_jobs():
        print 'Job Name:%s' % (job_instance.name)
        print '  Description:%s' % (job_instance.get_description())
        print '  Running:%s' % (job_instance.is_running())
        print '  Enabled:%s' % (job_instance.is_enabled())

if __name__ == '__main__':
    print get_job_details()
