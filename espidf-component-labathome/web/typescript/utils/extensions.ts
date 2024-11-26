export {};
declare global {
  interface Map<K, V> {
    getOrAdd(key: K, valueFactory: () => V): V;
    
  }
}
Map.prototype.getOrAdd = function <K, V>(this: Map<K, V>, key: K, valueFactory: () => V): V {
    if (this.has(key)) {
      return this.get(key) as V;
    }
    const value = valueFactory();
    this.set(key, value);
    return value;
  };
