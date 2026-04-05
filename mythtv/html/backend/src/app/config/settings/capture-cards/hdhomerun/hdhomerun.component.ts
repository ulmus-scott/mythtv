import { AfterViewInit, Component, ElementRef, Input, OnInit, ViewChild } from '@angular/core';
import { NgForm, FormsModule } from '@angular/forms';
import { CaptureCardService } from 'src/app/services/capture-card.service';
import { CaptureCardList, CaptureDevice, CaptureDeviceList, CardAndInput } from 'src/app/services/interfaces/capture-card.interface';
import { SetupService } from 'src/app/services/setup.service';
import { TranslatePipe } from '@ngx-translate/core';
import { ButtonModule } from 'primeng/button';
import { MessageModule } from 'primeng/message';
import { CheckboxModule } from 'primeng/checkbox';
import { InputNumberModule } from 'primeng/inputnumber';
import { ListboxModule } from 'primeng/listbox';

import { SharedModule } from 'primeng/api';
import { CardModule } from 'primeng/card';
import { CaptureCardsComponent } from '../capture-cards.component';

@Component({
    selector: 'app-hdhomerun',
    templateUrl: './hdhomerun.component.html',
    styleUrls: ['./hdhomerun.component.css'],
    imports: [FormsModule, CardModule, SharedModule, ListboxModule, InputNumberModule, CheckboxModule, MessageModule, ButtonModule, TranslatePipe]
})
export class HdhomerunComponent implements OnInit, AfterViewInit {

    @Input() card!: CardAndInput;
    @Input() cardList!: CaptureCardList;
    @ViewChild("hdhomerunform") currentForm!: NgForm;
    @ViewChild("top") topElement!: ElementRef;
    @Input() parent!: CaptureCardsComponent;
    @Input() tabIndex!: number;


    work = {
        isReady: false,
        successCount: 0,
        errorCount: 0,
    };

    captureDeviceList: CaptureDeviceList = {
        CaptureDeviceList: {
            CaptureDevices: [],
        }
    };

    selectedDevices: CaptureDevice[] = [];

    constructor(private captureCardService: CaptureCardService, public setupService: SetupService) { }

    ngOnInit(): void {
        // Get list of devices for dropdown list
        this.captureCardService.GetCaptureDeviceList('HDHOMERUN')
            .subscribe({
                next: data => {
                    this.captureDeviceList = data;
                    this.setupDevices();
                },
                error: (err: any) => {
                    console.log("GetCaptureDeviceList", err);
                    this.work.errorCount++;
                }
            });
        this.parent.children[this.tabIndex] = this;
    }

    dirty() {
        return this.currentForm.dirty;
    }

    ngAfterViewInit(): void {
        this.topElement.nativeElement.scrollIntoView({ behavior: "smooth", block: "start" });
        this.markPristine();
    }


    // After load of devices, make sure the current records are selected in list
    setupDevices(): void {
        // VideoDevice has the format "131AA209,131AA210"
        if (this.card.VideoDevice) {
            const devices = this.card.VideoDevice.split(',');
            devices.forEach(
                x => {
                    const dev = this.captureDeviceList.CaptureDeviceList.CaptureDevices.find(
                        y => x == y.VideoDevice.split(' ')[0]);
                    if (dev)
                        this.selectedDevices.push(dev);
                }
            );
        }
        this.work.isReady = true;
    }

    // After device update, update device-dependent fields
    updateDevices(): void {
        // Update device-dependent field
        let videoDevice = '';
        let videoDevices: string[] = [];
        this.selectedDevices.forEach(
            x => {
                videoDevices.push(x.VideoDevice.split(' ')[0]);
                this.card.SignalTimeout = x.SignalTimeout;
                this.card.ChannelTimeout = x.ChannelTimeout;
            }
        )
        videoDevices.sort();
        console.log(videoDevices);
        videoDevices.forEach(
            x => {
                if (videoDevice)
                    videoDevice = videoDevice + ',';
                videoDevice = videoDevice + x;
            }
        )
        this.card.VideoDevice = videoDevice;
    }

    // good response to add: {"int": 19}
    saveObserver = {
        next: (x: any) => {
            if (this.card.CardId && x.bool)
                this.work.successCount++;
            else if (!this.card.CardId && x.int) {
                this.work.successCount++;
                this.card.CardId = x.int;
            }
            else {
                this.work.errorCount++;
                this.currentForm.form.markAsDirty();
            }
        },
        error: (err: any) => {
            console.error(err);
            this.work.errorCount++;
            this.currentForm.form.markAsDirty();
        },
    };

    markPristine() {
        setTimeout(() => {
            this.currentForm.form.markAsPristine();
            this.parent.showDirty();
        }, 100);
    }

    saveForm() {
        this.work.successCount = 0;
        this.work.errorCount = 0;
        if (this.card.CardId) {
            // Update device and child devices
            this.cardList.CaptureCardList.CaptureCards.forEach(card => {
                if (card.CardId == this.card.CardId || card.ParentId == this.card.CardId) {
                    this.captureCardService.UpdateCaptureCard(card.CardId, 'videodevice', this.card.VideoDevice)
                        .subscribe(this.saveObserver);
                    this.captureCardService.UpdateCaptureCard(card.CardId, 'signal_timeout', String(this.card.SignalTimeout))
                        .subscribe(this.saveObserver);
                    this.captureCardService.UpdateCaptureCard(card.CardId, 'channel_timeout', String(this.card.ChannelTimeout))
                        .subscribe(this.saveObserver);
                    this.captureCardService.UpdateCaptureCard(card.CardId, 'dvb_eitscan', String(this.card.DVBEITScan ? '1' : '0'))
                        .subscribe(this.saveObserver);
                }
            });
        }
        else {
            this.captureCardService.AddCaptureCard(this.card).subscribe(this.saveObserver);
        }
    }

}
