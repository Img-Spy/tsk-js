declare module 'tsk-js' {
    export default TskJs.TSK;
}

declare namespace TskJs {
    class TSK {
        /**
         * Constructor
         * @param imgfile Path to an image file
         */
        constructor(imgfile: string);
 
        analyze(): ImgInfo;
        list(opts?: TskOptions): Array<ImgFile>;
        get(opts?: TskOptions): Buffer;
        timeline(cb?: TimelineCallback, opts?: TskOptions): Array<TimelineItem>;
        search(needle: string, cb?: SearchCallback, opts?: TskOptions): void;
    }

    export interface TskConstructor {
        new (imgfile: string): TSK;
        prototype: TSK;
    }

    export interface ImgInfo {
        type: "disk" | "partition";
        partitions?: Array<PartitionInfo>;
    }

    export interface TskOptions {
        imgaddr?: number;
        inode?: number;
    }

    export interface PartitionInfo {
        description: string;
        start: number;
        end: number;
        size:number;
        hasFs: boolean;
    }

    export interface ImgFile {
        path: string;
        name: string;
        allocated: boolean;
        type: "directory" | "virtual" | "register" | "unknown";
        metaType: "directory" | "virtual" | "register" | "unknown";
        inode: number;
        metaAddr: string;
        hasChildren?: boolean;
    }

    export type DiskAction = "access" | "modify" | "creation" | "change";
    export type TimelineCallback = (list: Array<TimelineItem>) => void;
    export type SearchCallback =
        (file: ImgFile, context: Buffer, index: number) => void;

    export interface TimelineItem extends ImgFile {
        actions: Array<DiskAction>;
        fileNameFlag: Boolean;
        date: Date;
    }
}