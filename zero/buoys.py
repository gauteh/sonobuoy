# -*- coding: utf-8 -*-
# Author: Gaute Hope <eg@gaute.vetsj.com> / 2012-04-10
#
# Defines buoys and addresses
#
# Version field is unused at the moment. But has been incremented
# at deployed version updates.

buoys = [ { 'id' :        1,
            'name' :      'One',
            'address' :   '04:3C:60',
            'enabled' :   True,
            'version' :   3,
            'getdata' :   True,
          },

          { 'id' :        2,
            'name' :      'Two',
            'address' :   '00:00:FF',
            'enabled' :   False,
            'version' :   3,
            'getdata' :   False,
          },

          { 'id' :        3,
            'name' :      'Three',
            'address' :   '04:3C:64',
            'enabled' :   False,
            'version' :   3,
            'getdata' :   True,
          },

          { 'id' :        4,
            'name' :      'Four',
            'address' :   '04:3C:93',
            'enabled' :   False,
            'version' :   2,
            'getdata' :   False,
          },

          { 'id' :        5,
            'name' :      'Five',
            'address' :   '03:FF:40',
            'enabled' :   False,
            'version' :   3,
            'getdata' :   True,
          },
        ]

