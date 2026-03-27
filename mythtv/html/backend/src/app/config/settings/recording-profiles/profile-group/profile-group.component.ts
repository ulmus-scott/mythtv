import { Component, HostListener, Input, OnInit } from '@angular/core';
import { NgForm, FormsModule } from '@angular/forms';
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import { Observable, of } from 'rxjs';
import { CaptureCardService } from 'src/app/services/capture-card.service';
import { RecProfileGroup } from 'src/app/services/interfaces/recprofile.interface';
import { SetupService } from 'src/app/services/setup.service';
import { RecordingProfilesComponent } from '../recording-profiles.component';
import { RecprofileComponent } from './recprofile/recprofile.component';
import { AccordionModule } from 'primeng/accordion';
import { MessageModule } from 'primeng/message';
import { NgIf, NgFor } from '@angular/common';
import { SharedModule } from 'primeng/api';
import { DialogModule } from 'primeng/dialog';
import { ButtonModule } from 'primeng/button';

@Component({
    selector: 'app-profile-group',
    templateUrl: './profile-group.component.html',
    styleUrls: ['./profile-group.component.css'],
    imports: [ButtonModule, DialogModule, FormsModule, SharedModule, NgIf, MessageModule, AccordionModule, NgFor, RecprofileComponent, TranslateModule]
})
export class ProfileGroupComponent implements OnInit {

    @Input() group!: RecProfileGroup;
    @Input() parent!: RecordingProfilesComponent;
    @Input() tabIndex!: number;

    currentTab: number = -1;
    deletedTab = -1;
    dirtyMessages: string[] = [];
    children: any[] = [];
    disabledTab: boolean[] = [];
    activeTab: boolean[] = [];
    readyCount = 0;
    displayNewDialog = false;
    newProfileName = '';
    successCount = 0;
    expectedCount = 0;
    errorCount = 0;


    displayDeleteThis: boolean[] = [];
    dirtyText = 'settings.common.unsaved';
    warningText = 'settings.common.warning';
    deletedText = 'settings.common.deleted';
    newText = 'settings.common.new';

    constructor(private captureCardService: CaptureCardService,
        public setupService: SetupService, private translate: TranslateService) {
        translate.get(this.dirtyText).subscribe(data => this.dirtyText = data);
        translate.get(this.warningText).subscribe(data => this.warningText = data);
        translate.get(this.deletedText).subscribe(data => this.deletedText = data);
        translate.get(this.newText).subscribe(data => this.newText = data);
    }

    ngOnInit(): void {
        // let tabNum = this.parent.groups.findIndex(entry => entry === this.group);
        // this.parent.profileGroups[tabNum] = this;
        this.parent.children[this.tabIndex] = this;
    }

    onClick(e: { index: number }) {
        console.log("onclick",e.index);
        this.onTabOpen(e);
    }

    onTabOpen(e: { index: number }) {
        // Get rid of successful delete when opening a new tab
        this.showDirty();
        // let form = this.setupService.getCurrentForm();
        // if (form != null)
        //     this.forms[e.index] = form;
        // this.setupService.setCurrentForm(null);
        this.currentTab = e.index;
        // This line removes "Unsaved Changes" from current tab header.
        // this.dirtyMessages[this.currentTab] = "";
        // This line supports showing "Unsaved Changes" on current tab header,
        // and you must comment the above line,
        // but the "Unsaved Changes" text does not go away after save, so it
        // is no good until we solve that problem.
        // (<NgForm>this.forms[e.index]).valueChanges!.subscribe(() => this.showDirty())
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

    // showDirty() {
    //     if (this.currentTab == -1
    //         || this.disabledTab[this.currentTab])
    //         return;
    //     if (this.forms[this.currentTab] && (<NgForm>this.forms[this.currentTab]).dirty)
    //         this.dirtyMessages[this.currentTab] = this.dirtyText;
    //     else if (!this.group.RecProfiles[this.currentTab].Id)
    //         this.dirtyMessages[this.currentTab] = this.newText;
    //     else
    //         this.dirtyMessages[this.currentTab] = "";
    // }

    newProfile() {
        this.displayNewDialog = false;
        for (let i = 0; i < this.activeTab.length; i++)
            this.activeTab[i] = false;
        this.group.RecProfiles.push({
            Id: 0,
            Name: this.newProfileName.trim(),
            VideoCodec: 'MPEG-4',
            AudioCodec: 'MP3',
            RecProfParams: []
        });
        let newTab = this.group.RecProfiles.length - 1;
        this.currentTab = newTab;
        this.showDirty();
    }

    delObserver = {
        next: (x: any) => {
            if (x.bool) {
                this.successCount++;
                if (this.deletedTab > -1) {
                    this.dirtyMessages[this.deletedTab] = this.deletedText;
                    this.disabledTab[this.deletedTab] = true;
                    this.activeTab[this.deletedTab] = false;
                    this.deletedTab = -1;
                }
            }
            else {
                this.errorCount++;
                this.deletedTab = -1;
            }
        },
        error: (err: any) => {
            console.error(err);
            this.errorCount++;
        },
    };


    deleteThis(index: number) {
        this.errorCount = 0;
        this.successCount = 0;
        this.expectedCount = 1;
        this.displayDeleteThis[index] = false;
        // To ensure delObserver flags correct item
        this.deletedTab = index;
        this.captureCardService.DeleteRecProfile(this.group.RecProfiles[index].Id)
            .subscribe(this.delObserver);
    }

    rejectName(): boolean {
        let trimmed = this.newProfileName.trim();
        return (trimmed == '' || this.group.RecProfiles.find(x => x.Name == trimmed) != undefined);
    }

    confirm(message?: string): Observable<boolean> {
        const confirmation = window.confirm(message);
        return of(confirmation);
    };

    allClean(): boolean {
        // let currentForm = this.setupService.getCurrentForm();
        if (this.children[this.currentTab] && (this.children[this.currentTab]).dirty()
            || this.dirtyMessages.find(element => element && element.length > 0)) {
            return false;
        }
        return true;
    }

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
