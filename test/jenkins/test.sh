#request
curl -s -u greg:<user token> localhost:8080/crumbIssuer/api/json
# answer
{"_class":"hudson.security.csrf.DefaultCrumbIssuer","crumb":"0160de4c4b2eaee8146184c18ea4781f","crumbRequestField":"Jenkins-Crumb"}

# request
curl -X POST -u greg:<user token> -H 'Jenkins-Crumb:0160de4c4b2eaee8146184c18ea4781f' 'localhost:8080/job/greg-karn-all/job/build?delay=0'
