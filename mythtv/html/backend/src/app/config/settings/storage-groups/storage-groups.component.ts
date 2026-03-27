import { Component, HostListener, OnInit, ViewEncapsulation } from '@angular/core';
import {  FormsModule } from '@angular/forms';
import { Router } from '@angular/router';
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import { Observable, of } from 'rxjs';
import { CanComponentDeactivate } from 'src/app/can-deactivate-guard.service';
import { StorageGroupDir } from 'src/app/services/interfaces/storagegroup.interface';
import { MythService } from 'src/app/services/myth.service';
import { SetupService } from 'src/app/services/setup.service';
import { SgroupComponent } from './sgroup/sgroup.component';
import {  NgIf } from '@angular/common';
import { AccordionModule } from 'primeng/accordion';
import { SharedModule } from 'primeng/api';
import { DialogModule } from 'primeng/dialog';
import { ButtonModule } from 'primeng/button';
import { CardModule } from 'primeng/card';


interface GroupCard {
    GroupName: string,
    LocalizedName: string,
    DirNames: string[],
};

@Component({
    selector: 'app-storage-groups',
    templateUrl: './storage-groups.component.html',
    styleUrls: ['./storage-groups.component.css'],
    encapsulation: ViewEncapsulation.None,
    imports: [
        CardModule,
        ButtonModule,
        DialogModule,
        FormsModule,
        SharedModule,
        AccordionModule,
        NgIf,
        TranslateModule,
        SgroupComponent,
    ]
})

export class StorageGroupsComponent implements OnInit, CanComponentDeactivate {

    children: SgroupComponent[] = [];
    dirtyMessages: string[] = [];
    currentTab: number = -1;
    activeTab: boolean[] = [];

    dirtyText = 'settings.common.unsaved';
    warningText = 'settings.common.warning';
    deletedText = 'settings.common.deleted';
    newText = 'settings.common.new';

    hostName: string = ""; // hostname of the backend server
    storageGroupDirs: StorageGroupDir[] = [];
    storageGroups: GroupCard[] = [
        // Below are the special groups from kSpecialGroups in MythTV, plus Default
        { GroupName: "Default", LocalizedName: "", DirNames: [] },
        { GroupName: "LiveTV", LocalizedName: "", DirNames: [] },
        { GroupName: "DB Backups", LocalizedName: "", DirNames: [] },
        { GroupName: "Videos", LocalizedName: "", DirNames: [] },
        { GroupName: "Trailers", LocalizedName: "", DirNames: [] },
        { GroupName: "Coverart", LocalizedName: "", DirNames: [] },
        { GroupName: "Fanart", LocalizedName: "", DirNames: [] },
        { GroupName: "Screenshots", LocalizedName: "", DirNames: [] },
        { GroupName: "Banners", LocalizedName: "", DirNames: [] },
        { GroupName: "Photographs", LocalizedName: "", DirNames: [] },
        { GroupName: "Music", LocalizedName: "", DirNames: [] },
        { GroupName: "MusicArt", LocalizedName: "", DirNames: [] },
    ];

    displayNewDlg = false;
    newGroupName = "";
    loadedCount = 0;


    constructor(private setupService: SetupService, private translate: TranslateService,
        private mythService: MythService, public router: Router) {
        this.setupService.setCurrentForm(null);
        this.mythService.GetHostName().subscribe(data => {
            this.hostName = data.String;
            this.loadGroups();
        });

        translate.get(this.dirtyText).subscribe(data => this.dirtyText = data);
        translate.get(this.warningText).subscribe(data => this.warningText = data);
        translate.get(this.deletedText).subscribe(data => this.deletedText = data);
        translate.get(this.newText).subscribe(data => this.newText = data);
        this.storageGroups.forEach(
            entry => translate.get("settings.sgroups.special." + entry.GroupName)
                .subscribe(data => entry.LocalizedName = data)
        );
    }

    loadGroups() {
        this.mythService.GetStorageGroupDirs({ HostName: this.hostName })
            .subscribe(data => {
                this.storageGroupDirs = data.StorageGroupDirList.StorageGroupDirs;
                this.storageGroupDirs.forEach(entry => {
                    let match = this.storageGroups.find(x => x.GroupName == entry.GroupName);
                    if (match)
                        match.DirNames.push(entry.DirName);
                    else {
                        this.storageGroups.push({
                            GroupName: entry.GroupName,
                            LocalizedName: entry.GroupName,
                            DirNames: [entry.DirName]
                        });
                    }

                });
                this.loadedCount++;
                for (let ix = 0; ix < this.children.length; ix++) {
                    if (this.children[ix]) {
                        this.children[ix].load();
                    }
                }
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

    // Temporary until onTabOpen and onTabClose are fixed
    onClick(e: { index: number }) {
        this.onTabOpen(e);
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

    newGroup() {
        this.displayNewDlg = false;
        let match = this.storageGroups.find(x => x.GroupName == this.newGroupName);
        if (match)
            return;
        this.storageGroups.push({
            GroupName: this.newGroupName,
            LocalizedName: this.newGroupName,
            DirNames: []
        });
        this.newGroupName = "";
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
