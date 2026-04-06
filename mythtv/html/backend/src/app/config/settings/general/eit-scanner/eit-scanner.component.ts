import { AfterViewInit, Component, Input, OnInit, ViewChild } from '@angular/core';
import { NgForm, FormsModule } from '@angular/forms';

import { MythService } from '../../../../services/myth.service';
import { SetupService } from '../../../../services/setup.service';
import { TranslatePipe } from '@ngx-translate/core';
import { ButtonModule } from 'primeng/button';
import { MessageModule } from 'primeng/message';

import { InputNumberModule } from 'primeng/inputnumber';
import { SharedModule } from 'primeng/api';
import { CardModule } from 'primeng/card';
import { SettingsComponent } from '../general-settings.component';

@Component({
    selector: 'app-eit-scanner',
    templateUrl: './eit-scanner.component.html',
    styleUrls: ['./eit-scanner.component.css'],
    imports: [FormsModule, CardModule, SharedModule, InputNumberModule, MessageModule, ButtonModule, TranslatePipe]
})
export class EitScannerComponent implements OnInit, AfterViewInit {

    @ViewChild("eitscanopt") currentForm!: NgForm;
    @Input() parent!: SettingsComponent;
    @Input() tabIndex!: number;


    successCount = 0;
    errorCount = 0;
    EITTransportTimeout = 5;
    EITCrawIdleStart = 60;
    EITScanPeriod = 15;

    constructor(public setupService: SetupService, private mythService: MythService) {
        this.getEITScanner();
    }

    ngOnInit(): void {
        this.parent.children[this.tabIndex] = this;
    }

    ngAfterViewInit() {
    }

    getEITScanner() {

        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "EITTransportTimeout", Default: "5" })
            .subscribe({
                next: data => this.EITTransportTimeout = Number(data.String),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "EITCrawIdleStart", Default: "60" })
            .subscribe({
                next: data => this.EITCrawIdleStart = Number(data.String),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "EITScanPeriod", Default: "15" })
            .subscribe({
                next: data => this.EITScanPeriod = Number(data.String),
                error: () => this.errorCount++
            });

    }

    dirty() {
        return this.currentForm.dirty;
    }

    eitObserver = {
        next: (x: any) => {
            if (x.bool)
                this.successCount++;
            else {
                this.errorCount++;
                if (this.currentForm)
                    this.currentForm.form.markAsDirty();
            }
        },
        error: (err: any) => {
            console.error(err);
            this.errorCount++;
            if (this.currentForm)
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
        this.successCount = 0;
        this.errorCount = 0;
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "EITTransportTimeout",
            Value: String(this.EITTransportTimeout)
        }).subscribe(this.eitObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "EITCrawIdleStart",
            Value: String(this.EITCrawIdleStart)
        }).subscribe(this.eitObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "EITScanPeriod",
            Value: String(this.EITScanPeriod)
        }).subscribe(this.eitObserver);
    }

}
