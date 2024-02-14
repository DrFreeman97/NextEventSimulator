from typing import Any
import networkx as nx
import graphviz


class SystemParameters:
    alpha = 0.8
    beta = 0.2
    u1 = 15
    u2 = 75
    thinkTime = 5000
    timeSlice = 3
    Sio1 = 40
    Sio2 = 180
    qio1 = 0.65 #route to io1
    qio2 = 0.25 # route to io2
    qoutd = 0.1*0.4 #go to delay station
    qouts = 0.1*0.6 # renter the system
    pass

class DiGraph():
   
   def __init__(self):
     self.graph = nx.digraph.DiGraph()
     self.viz = graphviz.Digraph()
     self.lastHead = ""
     self.lastTail = ""
     self.calls = 0
     pass

   def gen(self,headLabel, tailLabel, p):
    if (headLabel,tailLabel) in self.graph.edges:
      print("Warning redundant edge {} , {} , call N {}".format(headLabel,tailLabel,self.calls))
      pass
    if headLabel not in self.graph.nodes:
      print("Warning head {} not listed".format(headLabel))
      pass
    if tailLabel not in self.graph.nodes:
      print("Warning tail {} not listed".format(tailLabel))
      pass
    self.lastHead = headLabel
    self.lastTail = tailLabel 
    self.graph.add_edge(headLabel,tailLabel,weight=round(p,5))
    return (headLabel,tailLabel)

   def Graph(self):
     return self.graph
   
   
   def __and__(self,arg:tuple[str,str,float]):
     return self.gen(arg[0],arg[1],arg[2])
  
   
   def __getitem__(self,index):
     return self.graph.edges()[index]
   
   def add_edge(self,head,tail,p):
     self.gen(head() if callable(head) else head, tail() if callable(tail) else tail, p)
     pass


   
   def __call__(self, head,tail,pout,pin =0):
     if callable(head): 
       head = head()
       pass
     if callable(tail):
       tail = tail()
       pass
     self.add_edge(head,tail,pout)
     if pin > 0:
      self.add_edge(tail,head,pin)
      pass
     self.calls += 1
     return self
   
   def add_edges(self,edges: list[tuple]):
     for edge in edges:
       head = ""
       tail = ""
       if callable(edge[0]): head = edge[0]()
       else: head=edge[0]
       if callable(edge[1]): tail = edge[1]()
       else: tail = edge[1]
       self.gen(head,tail,edge[2])
       pass
     pass
   
   def add_node(self,node:str):
     self.graph.add_node(node)
     pass
   
   
   def last_tail(self): return self.lastTail
   def last_head(self) : return self.lastHead

   def gviz(self):
      graph = graphviz.Digraph()
      for node in self.graph.nodes:
        graph.node(node)
        pass
      for edge in self.graph.edges:
       graph.edge(edge[0],edge[1],str(self.graph.get_edge_data(edge[0],edge[1])["weight"]))
       pass
      return graph
   
   def fig(self):
     pos = nx.draw_planar(self.graph)
     fig = nx.draw_networkx_nodes(self.graph,pos,node_size=1000)
     fig = nx.draw_networkx_labels(self.graph,pos)
     fig = nx.draw_networkx_edges(self.graph,pos)
     fig = nx.draw_networkx_edge_labels(self.graph,pos)
     pass
   pass



class State:
    def __init__(self,Ndelay, Ncpu, Nio1, Nio2,cpuStage = 0) -> None:
        self.Ndelay = Ndelay
        self.Ncpu = Ncpu
        self.cpuStage = cpuStage
        self.Nio1 = Nio1
        self.Nio2 = Nio2
        self.state= [Ndelay,Ncpu,Nio1,Nio2,cpuStage]
        self.descriptor =["Ndelay","Ncpu","Nio1","Nio2","cpuStage"]
        pass

    def isValid(self)->bool:
        if self.Ncpu > 0:
            return (self.cpuStage == 1 or self.cpuStage == 2) and (self.Ncpu + self.Nio1 + self.Nio2 + self.Ndelay) == 3
        else:
            return (self.Ncpu + self.Nio1 + self.Nio2 + self.Ndelay) == 3
    
    def __str__(self) -> str:
        return ("{},{},{},{}".format(
            self.Ndelay,
            self.Ncpu if self.Ncpu == 0 else "{}.{}".format(self.Ncpu,self.cpuStage),
            self.Nio1,
            self.Nio2
        ))
    
    def __call__(self, index : int) -> str:
       
       pass
    
    def __getitem__(self, index:int):
       return self.state[index]

    def __len__(self):
       return len(self.state) - 1
       
    pass


class Transition:
   
   class TransitionType:
      UNKNOWN = -1
      CPU_TO_IO1 = 1
      CPU_TO_IO2 = 2
      IO2_TO_CPU = 4
      IO1_TO_CPU = 5
      DELAY_TO_CPU = 6
      CPU_TO_DELAY = 7
      CPU_TO_SELF = 8
      pass

   def __init__(self, head: State, tail : State) -> None:
      self.head = head
      self.tail = tail
      self.type = Transition.TransitionType.UNKNOWN
      self.detectType()
      pass
   
   def transitionIsValid(self):
      #check cpu stage
      if self.head.cpuStage != self.tail.cpuStage:
         if self.head.Ncpu < self.tail.Ncpu and self.tail.Ncpu > 0: return False
         pass
      changed = False 
      def validate_var(a , b):
         nonlocal changed 
         if abs(a - b ) > 1 : return False 
         if abs(a-b) == 1 and changed: return False
         if abs(a-b) == 0 : return True
         else: changed = True         
         return True
      for i in range(len(self.head)):
         if not validate_var(self.head[i],self.tail[i]): return False
         pass
      return True
   
   def detectMovement(self) -> tuple[str,str]:
      decremented = ""
      incremented = ""
      for i in range(len(self.head)):
         if self.head[i] != self.tail[i]:
            if self.head[i] > self.tail[i]:
               incremented = self.head.descriptor[i] if incremented == "" else "ERROR"               
               pass
            if self.head[i] < self.tail[i]:
               decremented = self.head.descriptor[i] if decremented == "" else "ERROR"
               pass
         pass
      return (decremented,incremented)

   def detectType(self):
      if not self.transitionIsValid() : return
      (decrement,increment)= self.detectMovement
      if decrement == "ERROR" or increment == "ERROR" : return 
      elif decrement == "Ncpu" and increment == "Nio1": self.type = Transition.TransitionType.CPU_TO_IO1
      elif decrement == "Ncpu" and increment == "Nio2" : self.type = Transition.TransitionType.CPU_TO_IO2
      elif decrement == "Nio1" and increment == "Ncpu" : self.type = Transition.TransitionType.IO1_TO_CPU
      elif decrement == "Nio2" and increment == "Ncpu" : self.type = Transition.TransitionType.IO2_TO_CPU
      elif decrement == "Ndelay" and increment == "Ncpu" : self.type = Transition.TransitionType.DELAY_TO_CPU
      elif decrement == "Ncpu" and increment == "Ndelay" : self.type = Transition.TransitionType.CPU_TO_DELAY
      else : self.type = Transition.TransitionType.CPU_TO_SELF
      pass
   
   def p(self):
      assert(self.type != Transition.TransitionType.UNKNOWN)
      pass
   
   def DelayToCpu(self):
      return self.head.Ndelay*(1/SystemParameters.thinkTime)
   
   def CpuL(self):
      l = 1/SystemParameters.u1 if self.head.cpuStage == 1 else 1/SystemParameters.u2
      if self.tail.Ncpu > 0:
         l = l * SystemParameters.alpha if self.tail.cpuStage == 1 else SystemParameters.beta
         pass
      return l
      

   def CpuToIo(self):      
      return self.CpuL()*SystemParameters.qio1 if self.type == Transition.TransitionType.CPU_TO_IO1 else self.CpuL()*SystemParameters.qio2
   
   def CpuToDelay(self):
      return self.CpuL()*SystemParameters.qoutd
   
   def CpuToSelf(self):
      return self.CpuL()*SystemParameters.qouts
   
   def IoToCpu(self):
      return (1/SystemParameters.Sio1) if self.type == Transition.TransitionType.IO1_TO_CPU else (1/SystemParameters.Sio2)

   pass


def stage_enumerator(stage: int) -> list[State]:
    Ndelay = 0
    Ncpu = 1
    Nio1 = 2
    Nio2 = 3
    cpuStage = stage
    stages = [0,0,0,0]
    result = []
    while stages[Ndelay] <= 3:
        for i in reversed(range(4)):
            stages[i] += 1
            if stages[i] <= 3: break
            elif i > 0 : stages[i] = 0
            pass
        state= State(stages[Ndelay], stages[Ncpu], stages[Nio1],stages[Nio2],stage)
        if state.isValid(): result.append(state)
        pass
    return result

if __name__ == "__main__":
    print(list(map(lambda x: str(x),stage_enumerator(1))))
    print(list(map(lambda x: str(x),stage_enumerator(2))))

    s1 = State(3,0,0,0)
    s2 = State(2,1,0,0,1)
    assert(s1.isAdjacent(s2))
    print("All tests passed")
    pass


