from django.http      import HttpResponse, HttpResponseRedirect
from django.shortcuts import render_to_response
from django.template  import Context, RequestContext

# zero
from ui   import *
from zero import *
from buoy import *
from ad   import *


def connect_zero ():
  m = ZeroUIManager ()
  m.setup_client ()

  try:
    m.connect ()
  except:
    print "Error: Could not connect to Zero manager. Is the service running?"

  return m

def overview (request):
  m = connect_zero ()
  z = m.get_zcliservice ()

  statuses = z.buoy_statuses ()

  return render_to_response ('overview.htm', Context({'statues' : statuses}))

def details (request, buoy):
  m = connect_zero ()

  return render_to_response ('details.htm', Context({ 'name' : buoy }))

