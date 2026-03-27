import { Component, HostListener, OnInit, ViewEncapsulation } from '@angular/core';
import { Router } from '@angular/router';
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import { Observable, of } from 'rxjs';
import { CaptureCardService } from 'src/app/services/capture-card.service';
import { ChannelService } from 'src/app/services/channel.service';
import { CaptureCardList, CardAndInput } from 'src/app/services/interfaces/capture-card.interface';
import { VideoSource, VideoSourceList } from 'src/app/services/interfaces/videosource.interface';
import { MythService } from 'src/app/services/myth.service';
import { SetupService } from 'src/app/services/setup.service';
import { ButtonModule } from 'primeng/button';
import { IconnectionComponent } from './iconnection/iconnection.component';
import { SharedModule } from 'primeng/api';
import { AccordionModule } from 'primeng/accordion';
import { CardModule } from 'primeng/card';

@Component({
    selector: 'app-input-connections',
    templateUrl: './input-connections.component.html',
    styleUrls: ['./input-connections.component.css'],
    encapsulation: ViewEncapsulation.None,
    imports: [
        CardModule,
        AccordionModule,
        SharedModule,
        IconnectionComponent,
        ButtonModule,
        TranslateModule,
    ]
})
export class InputConnectionsComponent implements OnInit {

    currentTab: number = -1;
    deletedTab = -1;
    dirtyMessages: string[] = [];
    children: IconnectionComponent[] = [];
    activeTab: boolean[] = [];
    readyCount = 0;

    dirtyText = 'settings.common.unsaved';
    warningText = 'settings.common.warning';

    m_hostName: string = ""; // hostname of the backend server
    m_CaptureCardList!: CaptureCardList;
    m_CaptureCardsFiltered: CardAndInput[] = [];
    m_CaptureCardList$!: Observable<CaptureCardList>;

    videoSourceList: VideoSourceList = {
        VideoSourceList: {
            VideoSources: [],
            AsOf: '',
            Version: '',
            ProtoVer: ''
        }
    };

    // Video Sources Indexed by id
    videoSourceLookup: VideoSource[] = [];

    constructor(private mythService: MythService, public router: Router,
        private captureCardService: CaptureCardService, private setupService: SetupService,
        private translate: TranslateService, private channelService: ChannelService) {
        // this.setupService.setCurrentForm(null);
        this.mythService.GetHostName().subscribe(data => {
            this.m_hostName = data.String;
            this.loadCards(true);
        });
        this.loadSources();
        translate.get(this.dirtyText).subscribe(data => this.dirtyText = data);
        translate.get(this.warningText).subscribe(data => this.warningText = data);
    }

    loadCards(doFilter: boolean) {
        // Get for all hosts in case they want to use delete all
        this.m_CaptureCardList$ = this.captureCardService.GetCaptureCardList('', '')
        this.m_CaptureCardList$.subscribe(data => {
            this.m_CaptureCardList = data;
            if (doFilter)
                this.filterCards();
            this.readyCount++;
        })
    }

    filterCards() {
        this.m_CaptureCardsFiltered
            = this.m_CaptureCardList.CaptureCardList.CaptureCards.filter
                (x => x.ParentId == 0 && x.HostName == this.m_hostName);
        this.dirtyMessages = [];
        this.children = [];
        this.activeTab = [];
        for (let x = 0; x < this.m_CaptureCardsFiltered.length; x++) {
            this.dirtyMessages.push('');
            this.activeTab.push(false);
        }
    }

    loadSources() {
        this.channelService.GetVideoSourceList()
            .subscribe(data => {
                this.videoSourceList = data;
                this.videoSourceList.VideoSourceList.VideoSources.unshift(<VideoSource>{
                    Id: 0, SourceName: "(None)", ScanFrequency: 0
                })
                this.videoSourceLookup = [];
                this.videoSourceList.VideoSourceList.VideoSources.forEach(data => {
                    this.videoSourceLookup[data.Id] = data;
                });
                this.readyCount++;
            });
    }

    ngOnInit(): void {
    }

    onTabOpen(e: { index: number }) {
        this.showDirty();
        this.currentTab = e.index;
    }

    onTabClose(e: any) {
        this.showDirty();
        this.currentTab = -1;
    }

    showDirty() {
        for (let ix = 0; ix < this.children.length; ix++) {
            if (this.children[ix]) {
                if (this.children[ix].dirty())
                    this.dirtyMessages[ix] = this.dirtyText;
                else
                    this.dirtyMessages[ix] = '';
            }
        }
    }
    confirm(message?: string): Observable<boolean> {
        const confirmation = window.confirm(message);
        return of(confirmation);
    };

    canDeactivate(): Observable<boolean> | boolean {
        if (this.children[this.currentTab] && (this.children[this.currentTab]).dirty()
            || this.dirtyMessages.find(element => element && element.length > 0)) {
            return this.confirm(this.warningText);
        }
        return true;
    }

    @HostListener('window:beforeunload', ['$event'])
    onWindowClose(event: any): void {
        if (this.children[this.currentTab] && (this.children[this.currentTab]).dirty()
            || this.dirtyMessages.find(element => element && element.length > 0)) {
            event.preventDefault();
            event.returnValue = false;
        }
    }



}
