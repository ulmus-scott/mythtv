
export interface FrontendStatus {
    Name: string,
    Version: string,
    State: any,
    ChapterTimes: string[],
    SubtitleTracks: any[],
    AudioTracks: any[],
}

export interface StatusResponse {
    FrontendStatus: FrontendStatus;
}

export interface BoolResponse {
    bool: boolean;
}

