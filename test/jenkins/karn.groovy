def run_stages(String job_name)
{
	//
	// Ensure job was configured as a multibranch pipeline and named
	// according the following convention :
	//   <arbritrary_string>-karn-<build_config_name>
	//
	// If this is the case, Jenkins will invoke our pipeline with the global
	// variable set to:
	//   <arbritrary_string>-karn-<build_config_name>/<branch_name>
	def job = job_name.tokenize('/')

	assert job.size() == 2
	job = job[0].tokenize('-')
	assert job.size() > 2
	assert job[job.size() - 2] == 'karn'
	job = job[job.size() - 1]

	stage('Config') {
		sh "make defconfig-${job}"
	}
	stage('Build') {
		sh 'make build'
	}
	stage('Test') {
		sh 'make test'
	}
	stage('Check') {
		sh 'make check'
	}
	stage('Coverage') {
		sh 'make cov'
	}
}

return this;
