using System.Collections.Generic;

namespace VMLib {

    // directed graph class
    public class Graph<T> where T : class {
        public List<T> Nodes = new List<T>();
        public List<Edge> Edges = new List<Edge>();
        
        public class Edge {
            public T Start;
            public T End;
        }

        // only add node if it is not already in the graph
        public void AddNode(T node) {
            if(!Nodes.Contains(node)) {
                Nodes.Add(node);
            }
        }

        // add edge to the graph and add the nodes if they are not already in the graph
        public void AddEdge(T start, T end) {
            Edges.Add(new Edge() { Start = start, End = end });
            if(!Nodes.Contains(start)) {
                Nodes.Add(start);
            }
            if(!Nodes.Contains(end)) {
                Nodes.Add(end);
            }
        }

        // create a list of all nodes that do not have any edges ending at that node
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

        // create a list containing the nodes sorted in topological order
        // without modifying the graph
        public List<T> TopologicalSort() {
            // currently uses kahn's algorithm (https://en.wikipedia.org/wiki/Topological_sorting#Kahn%27s_algorithm)
            // which modifies the graph, so create list of current nodes and edges
            // so they can be restored at the end of the function
            var n = new List<T>(Nodes);
            var e = new List<Edge>(Edges);

            // sorted nodes
            var ret = new List<T>();


            List<T> s = NodesNoInput();
            while(s.Count > 0) {
                var node = s[0];
                ret.Add(node);
                RemoveNode(node);
                s = NodesNoInput();
            }

            // nodes still remaining in graph, so not all were
            // sorted - restore nodes and edges and return empty list
            if(Nodes.Count > 0) {
                Nodes = n;
                Edges = e;
                VMCore.Log.Warn("Cyclic Microcode graph: did not execute");
                return new List<T>();
            }

            Nodes = n;
            Edges = e;
            return ret;
        }

        // remove node from graph and remove all edges that link to that node
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
