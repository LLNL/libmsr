const char NODESTATE_ATA[]="subscribe ns to newnodestate; string me; initialization {     me='%s'; } behavior {     if(ns.node==me) {         send(ns);     } } ";
