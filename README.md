# ARIAWorkspace
usage:

##clientOnPC 
the controller

Usage: ./clientOnPC -rh [hostname of HV] -rp [port of HV]

##serverOnHV
the server on HV,connects to HV through tcp [rhHV:rpHV],then forwards to FV1[rhFV1:rpFV1] ,and etc..

Usage: ./serverOnHV -rhHV [hostname of HV] -rpHV [port of HV] -rhFV1 [hostname of FV1] -rpFV1 [port of FV1]