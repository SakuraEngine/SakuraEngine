type Storage = any
type Query = any
export type Entity = number
export type CompArr<T> = {
    length: number,
    get : (self : CompArr<T>, index: number) -> T
}

declare class resource_handle
    function __tostring(self): string
    function __eq(self, other: resource_handle): boolean
    function resolve(self): any
    function is_resolved(self): boolean
    function get_resolved(self): any
    function unload(self): nil
end

export type View<T... = ...any> = {
    length : number,
    with : (self : View<T...>, entities: {number}, callback: ((view: View<T...>) -> ())) -> (),
    unpack : (self : View<T...>, index: number) -> (number, T...)
}

type Imgui = {
    test : () -> ()
}

type Skr = {
    imgui : Imgui,
    create_query : (storage: Storage, query: string) -> Query,
    print : (msg: string) -> (),
    iterate_query : (query: Query, callback: ((view: View) -> ())) -> ()
}

declare skr: Skr

type Game = {
    GetStorage : () -> Storage
}

declare game: Game

declare function newtable<T>(narr: number, nrec: number): {T}