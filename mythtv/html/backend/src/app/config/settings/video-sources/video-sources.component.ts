import { Component, HostListener, OnInit, ViewEncapsulation } from '@angular/core';
import { Router } from '@angular/router';
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import { Observable, of } from 'rxjs';
import { CanComponentDeactivate } from 'src/app/can-deactivate-guard.service';
import { ChannelService } from 'src/app/services/channel.service';
import { VideoSource, VideoSourceList } from 'src/app/services/interfaces/videosource.interface';
import { SetupService } from 'src/app/services/setup.service';
import { VsourceComponent } from './vsource/vsource.component';
import { AccordionModule } from 'primeng/accordion';
import { MessageModule } from 'primeng/message';
import { NgIf } from '@angular/common';
import { SharedModule } from 'primeng/api';
import { DialogModule } from 'primeng/dialog';
import { ButtonModule } from 'primeng/button';
import { CardModule } from 'primeng/card';

@Component({
    selector: 'app-video-sources',
    templateUrl: './video-sources.component.html',
    styleUrls: ['./video-sources.component.css'],
    encapsulation: ViewEncapsulation.None,
    imports: [
        CardModule,
        ButtonModule,
        DialogModule,
        SharedModule,
        NgIf,
        MessageModule,
        AccordionModule,
        VsourceComponent,
        TranslateModule,
    ]
})
export class VideoSourcesComponent implements OnInit, CanComponentDeactivate {

    currentTab: number = -1;
    deletedTab = -1;
    dirtyMessages: string[] = [];
    children: any[] = [];
    disabledTab: boolean[] = [];
    activeTab: boolean[] = [];
    displayDeleteThis: boolean[] = [];
    dirtyText = 'settings.common.unsaved';
    warningText = 'settings.common.warning';
    deletedText = 'settings.common.deleted';
    newText = 'settings.common.new';
    successCount: number = 0;
    expectedCount = 0;
    errorCount: number = 0;
    displayDeleteAll: boolean = false;
    deleteAll: boolean = false;

    videoSourceList: VideoSourceList = {
        VideoSourceList: {
            VideoSources: [],
            AsOf: '',
            Version: '',
            ProtoVer: ''
        }
    };

    constructor(public setupService: SetupService, private translate: TranslateService,
        private channelService: ChannelService, public router: Router) {
        // this.setupService.setCurrentForm(null);
        this.loadSources();
        translate.get(this.dirtyText).subscribe(data => this.dirtyText = data);
        translate.get(this.warningText).subscribe(data => this.warningText = data);
        translate.get(this.deletedText).subscribe(data => this.deletedText = data);
        translate.get(this.newText).subscribe(data => this.newText = data);
    }

    loadSources() {
        this.channelService.GetVideoSourceList()
            .subscribe(data => {
                this.videoSourceList = data;
                this.dirtyMessages = [];
                this.disabledTab = [];
                this.activeTab = [];
                this.displayDeleteThis = [];
                for (let x = 0; x < this.videoSourceList.VideoSourceList.VideoSources.length; x++) {
                    this.dirtyMessages.push('');
                    this.disabledTab.push(false);
                    this.activeTab.push(false);
                    this.displayDeleteThis.push(false);
                }
            });
    }

    ngOnInit(): void {
    }

    onTabOpen(e: { index: number }) {
        // Get rid of successful delete when opening a new tab
        if (this.successCount + this.errorCount >= this.expectedCount) {
            this.errorCount = 0;
            this.successCount = 0;
            this.expectedCount = 0;
        }
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
                else if (!this.videoSourceList.VideoSourceList.VideoSources[ix].Id)
                    this.dirtyMessages[ix] = this.newText;
                else
                    this.dirtyMessages[ix] = '';
            }
        }
    }


    newSource() {
        let newOne: VideoSource = <VideoSource>{
            Grabber: "eitonly",
            FreqTable: "default",
            ScanFrequency: 0,
            NITId: -1,
            BouquetId: 0,
            RegionId: 0,
            LCNOffset: 0,
            SourceName: ''
        };
        for (let i = 0; i < this.activeTab.length; i++)
            this.activeTab[i] = false;
        this.dirtyMessages.push(this.newText);
        this.disabledTab.push(false);
        this.activeTab.push(false);
        this.displayDeleteThis.push(false);
        this.videoSourceList.VideoSourceList.VideoSources.push(newOne);
    }

    deleteAllSources() {
        // Check if prior is finished by checking counts
        if (this.successCount + this.errorCount < this.expectedCount)
            return;
        this.errorCount = 0;
        this.successCount = 0;
        this.expectedCount = 0;
        this.displayDeleteAll = false;
        this.deletedTab = -1;
        this.deleteAll = true;
        for (let ix = 0; ix < this.videoSourceList.VideoSourceList.VideoSources.length; ix++) {
            if (!this.disabledTab[ix] && this.videoSourceList.VideoSourceList.VideoSources[ix].Id)
                this.deleteThis(ix);
        }
    }

    delObserver = {
        next: (x: any) => {
            if (x.bool) {
                this.successCount++;
                if (this.successCount == this.expectedCount) {
                    if (this.deleteAll) {
                        this.loadSources();
                        this.deleteAll = false;
                    }
                    else {
                        if (this.deletedTab > -1) {
                            this.dirtyMessages[this.deletedTab] = this.deletedText;
                            this.disabledTab[this.deletedTab] = true;
                            this.activeTab[this.deletedTab] = false;
                            this.deletedTab = -1;
                        }
                    }
                }
            }
            else {
                this.errorCount++;
                this.deletedTab = -1;
                this.deleteAll = false;
            }
        },
        error: (err: any) => {
            console.error(err);
            this.errorCount++;
            this.deleteAll = false;
        },
    };

    deleteThis(index: number) {
        let Id = this.videoSourceList.VideoSourceList.VideoSources[index].Id;
        if (!this.deleteAll) {
            // Check if prior is finished by checking counts
            if (this.successCount + this.errorCount < this.expectedCount)
                return;
            this.errorCount = 0;
            this.successCount = 0;
            this.expectedCount = 0;
            this.displayDeleteThis[index] = false;
            // To ensure delObserver flags correct item
            this.deletedTab = index;
        }
        this.expectedCount++;
        this.channelService.RemoveVideoSource(Id)
            .subscribe(this.delObserver);
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
