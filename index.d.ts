
declare module 'tsk-js' {
    export class TSK {
        constructor(imgfile: string);

        analyze(): ImgInfo;
        list(opts: TskOptions): Array<ImgFile>;
        get(opts?: TskOptions): Buffer;
        timeline(opts?: TskOptions, cb?: TimelineCallback): Array<TimelineItem>;
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
        name: string;
        allocated: boolean;
        type: "directory" | "virtual" | "register" | "unknown";
        inode: number;
        hasChildren?: boolean;
    }

    export type DiskAction = "";
    export type TimelineCallback = (list: Array<TimelineItem>) => void;

    export interface TimelineItem {
        path: string;
        name: string;
        allocated: boolean;
        type: "directory" | "virtual" | "register" | "unknown";
        inode: number;
        actions: Array<DiskAction>;
        date: Date;
    }
}
