import { HttpClient, HttpParams } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { Observable } from 'rxjs';
import { BoolResponse, StatusResponse } from '../interfaces/status.interface';

@Injectable({
    providedIn: 'root',
})
export class FrontendService {
    constructor(private httpClient: HttpClient) { }

    public GetStatus(): Observable<StatusResponse> {
        return this.httpClient.get<StatusResponse>('./Frontend/GetStatus');
    }

    public SendAction(action: string): Observable<BoolResponse> {
        return this.httpClient.post<BoolResponse>('./Frontend/SendAction', { Action: action })
    }

    public GetSetting(key: string, defaultval : string): Observable<{ String: string }> {
        let params = new HttpParams()
            .set("Key", key)
            .set("Default", defaultval);
        return this.httpClient.get<{ String: string }>('./Frontend/GetSetting', { params});
    }

}
