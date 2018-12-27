using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace VMLib {
    public class Graph<T> where T : class {
        public List<T> Nodes = new List<T>();
        public List<Edge> Edges = new List<Edge>();
        
        public class Edge {
            public T Start;
            public T End;
        }

        public void AddEdge(T start, T end) {
            Edges.Add(new Edge() { Start = start, End = end });
            if(!Nodes.Contains(start)) {
                Nodes.Add(start);
            }
            if(!Nodes.Contains(end)) {
                Nodes.Add(end);
            }
        }

        public List<T> NodesNoInput() {
            var ret = new List<T>();
            foreach(var node in Nodes) {
                var incoming = false;
                foreach(var edge in Edges) {
                    if(edge.End == node) {
                        incoming = true;
                    }
                }
                if(!incoming) {
                    ret.Add(node);
                }
            }
            return ret;
        }

        public List<T> TopologicalSort() {
            var ret = new List<T>();
            List<T> s = NodesNoInput();
            while(s.Count > 0) {
                var node = s[0];
                ret.Add(node);
                RemoveNode(node);
                s = NodesNoInput();
            }
            if(Nodes.Count > 0) {
                VMCore.Log.Warn("Cyclic Microcode graph: did not execute");
                return new List<T>();
            }
            return ret;
        }

        public void RemoveNode(T node) {
            for(int i = Edges.Count - 1; i >= 0; i--) {
                var edge = Edges[i];
                if(edge.Start == node || edge.End == node) {
                    Edges.RemoveAt(i);
                }
            }
            Nodes.Remove(node);
        }
    }
}
