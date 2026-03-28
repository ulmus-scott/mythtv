import { Component, HostListener, OnInit, ViewEncapsulation } from '@angular/core';
import { FormsModule } from '@angular/forms';
import { Router } from '@angular/router';
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import { Observable, of } from 'rxjs';
import { CanComponentDeactivate } from 'src/app/can-deactivate-guard.service';
import { CaptureCardService } from 'src/app/services/capture-card.service';
import { CaptureCardList, CardAndInput, CardType, CardTypeList, DiseqcTreeList } from 'src/app/services/interfaces/capture-card.interface';
import { MythService } from 'src/app/services/myth.service';
import { SetupService } from 'src/app/services/setup.service';
import { FirewireComponent } from './firewire/firewire.component';
import { VboxComponent } from './vbox/vbox.component';
import { SatipComponent } from './satip/satip.component';
import { HdpvrComponent } from './hdpvr/hdpvr.component';
import { V4l2Component } from './v4l2/v4l2.component';
import { DemoComponent } from './demo/demo.component';
import { ImportComponent } from './import/import.component';
import { IptvComponent } from './iptv/iptv.component';
import { HdhomerunComponent } from './hdhomerun/hdhomerun.component';
import { ExternalComponent } from './external/external.component';
import { DvbComponent } from './dvb/dvb.component';
import { CetonComponent } from './ceton/ceton.component';
import { AccordionModule } from 'primeng/accordion';
import { MessageModule } from 'primeng/message';

import { SharedModule } from 'primeng/api';
import { ListboxModule } from 'primeng/listbox';
import { DialogModule } from 'primeng/dialog';
import { ButtonModule } from 'primeng/button';
import { CardModule } from 'primeng/card';

@Component({
    selector: 'app-capture-cards',
    templateUrl: './capture-cards.component.html',
    styleUrls: ['./capture-cards.component.css'],
    encapsulation: ViewEncapsulation.None,
    imports: [
        CardModule,
        ButtonModule,
        DialogModule,
        ListboxModule,
        FormsModule,
        SharedModule,
        MessageModule,
        AccordionModule,
        CetonComponent,
        DvbComponent,
        ExternalComponent,
        HdhomerunComponent,
        IptvComponent,
        ImportComponent,
        DemoComponent,
        V4l2Component,
        HdpvrComponent,
        SatipComponent,
        VboxComponent,
        FirewireComponent,
        TranslateModule
    ]
})
export class CaptureCardsComponent implements OnInit, CanComponentDeactivate {

    static supportedCardTypes = [
        'CETON',
        'DVB',
        'EXTERNAL',
        'HDHOMERUN',
        'FREEBOX',
        'IMPORT',
        'DEMO',
        'V4L2ENC',
        'HDPVR',
        'SATIP',
        'VBOX',
        'FIREWIRE'
    ];

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

    m_hostName: string = ""; // hostname of the backend server
    m_CaptureCardList!: CaptureCardList;
    m_CaptureCardsFiltered!: CardAndInput[];
    m_CaptureCardList$!: Observable<CaptureCardList>;
    diseqcTreeList!: DiseqcTreeList;
    displayModal: boolean = false;
    selectedCardType: CardType = { CardType: "", Description: "" };
    displayDeleteAllonHost: boolean = false;
    displayDeleteAll: boolean = false;
    successCount: number = 0;
    expectedCount = 0;
    errorCount: number = 0;
    deleteAll: boolean = false;

    cardTypes: CardType[] = [];

    constructor(private mythService: MythService, public router: Router,
        private captureCardService: CaptureCardService, public setupService: SetupService,
        private translate: TranslateService) {
        this.mythService.GetHostName().subscribe(data => {
            this.m_hostName = data.String;
            this.loadCards(true);
        });
        translate.get(this.dirtyText).subscribe(data => this.dirtyText = data);
        translate.get(this.warningText).subscribe(data => this.warningText = data);
        translate.get(this.deletedText).subscribe(data => this.deletedText = data);
        translate.get(this.newText).subscribe(data => this.newText = data);

        this.captureCardService.GetCardTypeList().subscribe(data => {
            data.CardTypeList.CardTypes.forEach(entry => {
                if (CaptureCardsComponent.supportedCardTypes.indexOf(entry.CardType) >= 0)
                    this.cardTypes.push(entry);
            });
            this.cardTypes = [...this.cardTypes];
        });
    }

    loadCards(doFilter: boolean) {
        // Get for all hosts in case they want to use delete all
        this.m_CaptureCardList$ = this.captureCardService.GetCaptureCardList('', '')
        this.m_CaptureCardList$.subscribe(data => {
            this.m_CaptureCardList = data;
            if (doFilter)
                this.filterCards();
        })
    }

    filterCards() {
        this.m_CaptureCardsFiltered
            = this.m_CaptureCardList.CaptureCardList.CaptureCards.filter
                (x => x.ParentId == 0 && x.HostName == this.m_hostName);
        this.dirtyMessages = [];
        this.children = [];
        this.disabledTab = [];
        this.activeTab = [];
        this.displayDeleteThis = [];
        for (let x = 0; x < this.m_CaptureCardsFiltered.length; x++) {
            this.dirtyMessages.push('');
            this.disabledTab.push(false);
            this.activeTab.push(false);
            this.displayDeleteThis.push(false);
        }
    }

    ngOnInit(): void {
        this.loadDiseqc();
    }

    showDirty() {
        for (let ix = 0; ix < this.children.length; ix++) {
            if (this.children[ix]) {
                if (this.children[ix].dirty())
                    this.dirtyMessages[ix] = this.dirtyText;
                else if (!this.m_CaptureCardsFiltered[ix].CardId)
                    this.dirtyMessages[ix] = this.newText;
                else
                    this.dirtyMessages[ix] = '';
            }
        }
    }


    loadDiseqc() {
        // Get DiseqcTree list
        this.captureCardService.GetDiseqcTreeList()
            .subscribe({
                next: data => {
                    this.diseqcTreeList = data;
                },
                error: (err: any) => {
                    console.log("GetDiseqcTreeList", err);
                    this.errorCount++;
                }
            })
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

    newCard() {
        this.displayModal = false;
        let newOne: CardAndInput = <CardAndInput>{
            CardType: this.selectedCardType.CardType,
            HostName: this.m_hostName,
            ChannelTimeout: 3000,
            SignalTimeout: 1000
        };
        // Update non-standard defaults on some card types.
        switch (newOne.CardType) {
            case "EXTERNAL":
                newOne.ChannelTimeout = 20000;
                break;
            case "FREEBOX":
                newOne.VideoDevice = "http://mafreebox.freebox.fr/freeboxtv/playlist.m3u"
                newOne.ChannelTimeout = 30000;
                break;
            case "SATIP":
                newOne.DVBDiSEqCType = 1;
        }
        for (let i = 0; i < this.activeTab.length; i++)
            this.activeTab[i] = false;
        this.dirtyMessages.push(this.newText);
        this.disabledTab.push(false);
        this.activeTab.push(false);
        this.displayDeleteThis.push(false);
        this.m_CaptureCardsFiltered.push(newOne);
        this.selectedCardType = { CardType: "", Description: "" };
    }

    delObserver = {
        next: (x: any) => {
            if (x.bool) {
                this.successCount++;
                if (this.successCount == this.expectedCount) {
                    if (this.deleteAll) {
                        this.loadCards(true);
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
        let cardId = this.m_CaptureCardsFiltered[index].CardId;
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
        // delete any child cards. This only happens for a card that
        // was added before this session and had children created by the
        // input setup or automatically during recording.
        this.m_CaptureCardList.CaptureCardList.CaptureCards.forEach(card => {
            if (card.ParentId == cardId) {
                console.log("DeleteThis (parent):", card.CardId);
                this.expectedCount++;
                this.captureCardService.DeleteCaptureCard(card.CardId)
                    .subscribe(this.delObserver);
            }
        });
        // Delete any diseqc tree attached to the card
        // this.deleteDiseqc(this.m_CaptureCardsFiltered[index].DiSEqCId);
        this.m_CaptureCardsFiltered[index].DiSEqCId = 0;
        // Delete this card. Needs to be separate in case this card was added
        // during this session, then it would not be in the m_CaptureCardList.
        console.log("DeleteThis:", cardId);
        this.expectedCount++;
        this.captureCardService.DeleteCaptureCard(cardId)
            .subscribe(this.delObserver);
    }

    deleteAllOnHost() {
        // Check if prior is finished by checking counts
        if (this.successCount + this.errorCount < this.expectedCount)
            return;
        this.errorCount = 0;
        this.successCount = 0;
        this.expectedCount = 0;
        this.displayDeleteAllonHost = false;
        this.deletedTab = -1;
        this.deleteAll = true;
        for (let ix = 0; ix < this.m_CaptureCardsFiltered.length; ix++) {
            if (!this.disabledTab[ix] && this.m_CaptureCardsFiltered[ix].CardId)
                this.deleteThis(ix);
        }
    }

    deleteAllOnAllHosts() {
        // Check if prior is finished by checking counts
        if (this.successCount + this.errorCount < this.expectedCount)
            return;
        this.displayDeleteAll = false;
        this.errorCount = 0;
        this.successCount = 0;
        this.expectedCount = 0;
        this.deletedTab = -1;
        this.deleteAll = true;
        this.expectedCount++;
        this.captureCardService.DeleteAllCaptureCards()
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
